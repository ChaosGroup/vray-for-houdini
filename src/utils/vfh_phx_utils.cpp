//
// Copyright (c) 2015-2017, Chaos Software Ltd
//
// V-Ray For Houdini
//
// ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
//
// Full license text:
//   https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
//

#include "vfh_phx_utils.h"

#include <utils.h>
#include <misc.h>

#include <vassert.h>

UT_StringArray VRayForHoudini::PhxChannelsUtils::loadChannelsNames(const char* loadPath)
{
	using namespace VRayForHoudini::PhxChannelsUtils;

	UT_StringArray channels;

	int chanIndex = 0;
	int isChannelVector3D = 0;
	char chanName[MAX_CHAN_MAP_LEN];
	while (1 == aurGet3rdPartyChannelName(chanName, MAX_CHAN_MAP_LEN, &isChannelVector3D, loadPath, chanIndex++)) {
		channels.append(chanName);
	}

	return channels;
}

int VRayForHoudini::PhxAnimUtils::evalCacheFrame(fpreal frame, exint max_length, fpreal play_speed, exint anim_mode, fpreal t2f, exint play_at, exint load_nearest, exint read_offset)
{
	const exint animLen = max_length;
	const float fractionalLen = animLen * play_speed;

	switch (AnimMode(anim_mode)) {
	case AnimMode::directIndex: {
		frame = t2f;
		break;
	}
	case AnimMode::standard: {
		frame = play_speed * (frame - play_at);

		if (fractionalLen > 1e-4f) {
			if (frame < 0.f || frame > fractionalLen) {
				if (load_nearest) {
					frame = VUtils::clamp(frame, 0.f, fractionalLen);
				} else {
					frame = INT_MIN;
				}
			}
		}

		frame += read_offset;
		break;
	}
	case AnimMode::loop: {
		frame = play_speed * (frame - play_at);

		if (fractionalLen > 1e-4f) {
			while (frame < 0) {
				frame += fractionalLen;
			}
			while (frame > fractionalLen) {
				frame -= fractionalLen;
			}
		}

		frame += read_offset;
		break;
	}
	default:
		break;
	}

	return frame;
}

void VRayForHoudini::PhxAnimUtils::evalPhxPattern(QString &path, exint frame)
{
	using namespace VRayForHoudini::PhxChannelsUtils;

	QString cacheFrameS = QString::number(frame);
	QRegularExpressionMatch m = phxFramePattern.match(path);
	if (m.hasMatch()) {
		QString matched = m.captured();
		cacheFrameS = cacheFrameS.rightJustified(matched.size(), '0');
		path.replace(phxFramePattern, cacheFrameS);
	}
}

void VRayForHoudini::PhxAnimUtils::hou2PhxPattern(QString &path)
{
	using namespace VRayForHoudini::PhxChannelsUtils;

	QRegularExpressionMatch m = houFramePattern.match(path);
	if (m.hasMatch()) {
		int padding = m.captured().remove(0, 2).toInt();
		QString phxPattern(padding, '#');
		path.replace(houFramePattern, phxPattern);
	}
}