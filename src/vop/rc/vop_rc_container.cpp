//
// Copyright (c) 2015-2018, Chaos Software Ltd
//
// V-Ray For Houdini
//
// ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
//
// Full license text: https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
//

#include "vop_rc_container.h"

using namespace VRayForHoudini;

void VOP::RenderChannelsContainer::setPluginType()
{
	pluginType = VRayPluginType::SETTINGS;
	pluginID   = SL("SettingsRenderChannels");
}

const char* VOP::RenderChannelsContainer::inputLabel(unsigned idx) const
{
	return getCreateSocketLabel(idx, "Channel %i", idx+1);
}

int VOP::RenderChannelsContainer::getInputFromName(const UT_String &in) const
{
	return getInputFromNameSubclass(in);
}

void VOP::RenderChannelsContainer::getInputNameSubclass(UT_String &in, int idx) const
{
	in = getCreateSocketToken(idx, "chan_%i", idx+1);
}

int VOP::RenderChannelsContainer::getInputFromNameSubclass(const UT_String &in) const
{
	int inIdx = -1;

	if (in.startsWith("chan_")) {
		sscanf(in.buffer(), "chan_%i", &inIdx);
	}

	return inIdx-1;
}

void VOP::RenderChannelsContainer::getInputTypeInfoSubclass(VOP_TypeInfo &type_info, int idx)
{
	type_info.setType(VOP_TYPE_VOID);
}

void VOP::RenderChannelsContainer::getAllowedInputTypeInfosSubclass(unsigned idx, VOP_VopTypeInfoArray &type_infos)
{
	type_infos.append(VOP_TypeInfo(VOP_TYPE_VOID));
}

unsigned VOP::RenderChannelsContainer::getNumVisibleInputs() const
{
	int max = nInputs();

	// Make sure there is always exactly one unconnected unordered input
	// visible.
	if (max < orderedInputs()) {
		max = orderedInputs();
	}

	return max + 1;
}

unsigned VOP::RenderChannelsContainer::orderedInputs() const
{
	return 1;
}

OP::VRayNode::PluginResult VOP::RenderChannelsContainer::asPluginDesc(Attrs::PluginDesc &pluginDesc, VRayExporter &exporter)
{
	pluginDesc.pluginID = pluginID;
	pluginDesc.pluginName = SL("RenderChannels");

	// Last is always not connected.
	const int channelsCount = nInputs();

	ShaderExporter &shaderExporter = exporter.getShaderExporter();
	UT_String sockName;

	for (int i = 1; i <= channelsCount; ++i) {
		sockName.sprintf("chan_%i", i);

		const VRay::PluginRef chanPlugin = shaderExporter.exportConnectedSocket(*this, sockName);
		if (chanPlugin.isEmpty()) {
			Log::getLog().error("\"%s\": Failed to export render channel node connected to \"%s\"!",
			                    getFullPath().buffer(), sockName.buffer());
		}
	}

	return PluginResultContinue;
}
