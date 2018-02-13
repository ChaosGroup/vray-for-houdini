//
// Copyright (c) 2015-2017, Chaos Software Ltd
//
// V-Ray For Houdini
//
// ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
//
// Full license text:
//  https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
//

#include "vfh_vray_cloud.h"
#include "vfh_log.h"
#include "vfh_defines.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegExp>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QQueue>

#include <RE/RE_Window.h>

using namespace VRayForHoudini;
using namespace Cloud;

/// JSON config key for the executable file path.
static const QString keyExecutable("executable");

/// Resolved V-Ray Cloud Client binary file path.
static QString vrayCloudClient;

/// Flag indicatig that we've already tried to resolved
/// V-Ray Cloud Client binary file path.
static int vrayCloudClientChecked(false);

/// Not allowed characters regular expression.
static QRegExp cloudNameFilter("[^a-zA-Z\\d\\s\\_\\-.,\\(\\)\\[\\]]");

/// Only letters, digits, spaces and _-.,()[] allowed.
/// NOTE: QString::remove is non-const.
static QString filterName(QString value)
{
	return value.remove(cloudNameFilter);
}

/// Parses vcloud.json and extracts executable file path.
static void findVRayCloudClient()
{
	if (vrayCloudClientChecked)
		return;

	QString cgrHome; 
#ifdef _WIN32
	const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment();
	cgrHome = env.value("APPDATA");
#else
	cgrHome = QDir::homePath();
#endif

	QStringList vcloudDirList;
	vcloudDirList << cgrHome;
#ifdef _WIN32
	vcloudDirList << "Chaos Group";
#else
	vcloudDirList << ".ChaosGroup";
#endif
	vcloudDirList << "vcloud";
	vcloudDirList << "client";
	vcloudDirList << "vcloud.json";

	const QString vcloudFilePath = vcloudDirList.join(QDir::separator());

	QFileInfo vcloudFileInfo(vcloudFilePath);
	if (vcloudFileInfo.exists()) {
		QFile vcloudFile(vcloudFileInfo.absoluteFilePath());
		if (vcloudFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
			QJsonDocument vcloudJson = QJsonDocument::fromJson(vcloudFile.readAll());
			vcloudFile.close();

			if (vcloudJson.isObject()) {
				const QJsonObject vcloudObject = vcloudJson.object();
				if (vcloudObject.contains(keyExecutable)) {
					vrayCloudClient = vcloudObject[keyExecutable].toString();
				}
			}
		}
	}

	if (vrayCloudClient.isEmpty()) {
		Log::getLog().error("V-Ray Cloud Client is not found! Please, register at https://vray.cloud!");
	}

	vrayCloudClientChecked = true;
}

JobFilePath::JobFilePath(const QString &filePath)
	: filePath(filePath)
{}

JobFilePath::~JobFilePath()
{
	removeFilePath(filePath);
}

QString JobFilePath::createFilePath()
{
	QString tmpFilePath;

	QFileInfo tempDir(QDir::tempPath());
	if (!tempDir.isDir() || !tempDir.isWritable()) {
		Log::getLog().error("Temporary location \"%s\" is not writable!",
							_toChar(tempDir.absolutePath()));
	}
	else {
		QFileInfo tempFile(tempDir.absoluteFilePath(), "vfhVRayCloud.vrscene");
		if (tempFile.exists()) {
			removeFilePath(tempFile.absoluteFilePath());
		}

		tmpFilePath = tempFile.absoluteFilePath();
	}

	return tmpFilePath;
}

void JobFilePath::removeFilePath(const QString &filePath)
{
	const int removeRes = QFile::remove(filePath);
	if (!removeRes) {
		Log::getLog().error("Failed to remove \"%s\"!", _toChar(filePath));
	}
}

Job::Job(const QString &sceneFile)
	: sceneFile(sceneFile)
{}

void Job::setProject(const QString &value)
{
	project = filterName(value);
}

void Job::setName(const QString &value)
{
	name = filterName(value);
}

void Job::toArguments(const Job &job, QStringList &arguments)
{
	arguments << "--project" << job.getProject();
	arguments << "--name" << job.getName();
	arguments << "--sceneFile" << job.sceneFile;

	arguments << "--renderMode" << job.renderMode;

	arguments << "--width" << QString::number(job.width);
	arguments << "--height" << QString::number(job.height);

	if (job.animation) {
		arguments << "--animation";
		arguments << "--frameRange" << QString::number(job.frameRange.first) << QString::number(job.frameRange.second);
		arguments << "--frameStep" << QString::number(job.frameStep);
	}

	if (job.ignoreWarnings) {
		arguments << "--ignoreWarnings";
	}
	if (job.noProgress) {
		arguments << "--no-progress";
	}
	if (job.vr) {
		arguments << "--vr";
	}
	if (!job.colorCorrectionsFile.isEmpty()) {
		arguments << "--colorCorrectionsFile" << job.colorCorrectionsFile;
	}
}

class CloudWindow
	: public QMainWindow
{
	Q_OBJECT

public:
	struct CloudCommand {
		QString command;
		QStringList arguments; 
	};

	typedef QQueue<CloudCommand> CloudCommands;

	CloudWindow(CloudWindow* &cloudWindowInstance, const QString &jobFile, QWidget *parent)
		: QMainWindow(parent)
		, jobFile(jobFile)
		, cloudWindowInstance(cloudWindowInstance)
	{
		setWindowTitle("V-Ray Cloud Client");
		setAttribute(Qt::WA_DeleteOnClose);

		proc.setProcessChannelMode(QProcess::MergedChannels);

		connect(&proc, SIGNAL(readyReadStandardOutput()),
				this, SLOT(onProcStdOutput()));

		connect(&proc, SIGNAL(readyReadStandardError()),
				this, SLOT(onProcStdError()));

		connect(&proc, SIGNAL(error(QProcess::ProcessError)),
				this, SLOT(onProcError()));

		connect(&proc, SIGNAL(finished(int)),
				this, SLOT(onProcFinished()));

		editor = new QPlainTextEdit(this);
		editor->setReadOnly(true);

		progress = new QProgressBar(this);

		// Busy progress.
		progress->setMinimum(0);
		progress->setMaximum(0);

		stopButton = new QPushButton("Abort", this);

		connect(stopButton, SIGNAL(clicked()),
				this, SLOT(onPressTerminate()));

		QHBoxLayout *hlayout = new QHBoxLayout;
		hlayout->addWidget(progress);
		hlayout->addWidget(stopButton);

		QVBoxLayout *vlayout = new QVBoxLayout;
		vlayout->setMargin(5);
		vlayout->addWidget(editor);
		vlayout->addLayout(hlayout);

		QWidget *centralWidget = new QWidget(this);
		centralWidget->setLayout(vlayout);

		setCentralWidget(centralWidget);

		resize(640, 480);
	}

	~CloudWindow() VRAY_DTOR_OVERRIDE {
		cloudWindowInstance = nullptr;
	}

	void uploadScene(const Job &job) {
		{
			CloudCommand cmd;
			cmd.command = vrayCloudClient;
			cmd.arguments << "project" << "create" << "--name" << job.getProject();

			commands.enqueue(cmd);
		}
		{
			CloudCommand cmd;
			cmd.command = vrayCloudClient;
			cmd.arguments << "job" << "submit";

			Job::toArguments(job, cmd.arguments);

			commands.enqueue(cmd);
		}

		executeVRayCloudClient();
	}

	void executeVRayCloudClient() {
		if (commands.empty())
			return;

		const CloudCommand &cmd = commands.dequeue();

		Log::getLog().info("Calling V-Ray Cloud Client: %s", _toChar(cmd.arguments.join(" ")));

		proc.start(cmd.command, cmd.arguments, QIODevice::ReadOnly);
	}

	void uploadCompleted() const {
		progress->hide();
		stopButton->hide();
	}

private Q_SLOTS:
	void onPressTerminate() {
		commands.clear();
		proc.terminate();
	}

	void onProcError() {
		commands.clear();
		uploadCompleted();
	}

	void onProcFinished() {
		if (commands.empty()) {
			uploadCompleted();
			return;
		}

		executeVRayCloudClient();
	}

	void onProcStdError() {
		appendText(proc.readAllStandardError());
	}

	void onProcStdOutput() {
		appendText(proc.readAllStandardOutput());
	}

protected:
	void appendText(const QByteArray &data) const {
		QString text(data);
		editor->appendPlainText(text.simplified());
	}

	QPlainTextEdit *editor = nullptr;
	QProgressBar *progress = nullptr;
	QPushButton *stopButton = nullptr;

	QProcess proc;
	CloudCommands commands;

private:
	JobFilePath jobFile;
	CloudWindow* &cloudWindowInstance;
};

static CloudWindow *cloudWindowInstance = nullptr;

int VRayForHoudini::Cloud::submitJob(const Job &job)
{
	findVRayCloudClient();

	if (vrayCloudClient.isEmpty())
		return false;

	Log::getLog().info("Using V-Ray Cloud Client: \"%s\"", _toChar(vrayCloudClient));

	if (!cloudWindowInstance) {
		cloudWindowInstance = new CloudWindow(cloudWindowInstance, job.sceneFile, RE_Window::mainQtWindow());
		cloudWindowInstance->show();
		cloudWindowInstance->uploadScene(job);
	}

	return true;
}

#ifndef Q_MOC_RUN
#include <vfh_vray_cloud.cpp.moc>
#endif
