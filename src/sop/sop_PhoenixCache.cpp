//
// Copyright (c) 2015-2017, Chaos Software Ltd
//
// V-Ray For Houdini
//
// ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
//
// Full license text: https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
//

#ifdef CGR_HAS_AUR

#include "sop_PhoenixCache.h"
#include "vfh_prm_templates.h"

using namespace VRayForHoudini;
using namespace SOP;

PhxShaderCache::PhxShaderCache(OP_Network *parent, const char *name, OP_Operator *entry)
	: NodePackedBase("VRayVolumeGridRef", parent, name, entry)
{}

void PhxShaderCache::setPluginType()
{
	pluginType = VRayPluginType::GEOMETRY;
	pluginID   = "PhxShaderCache";
}

void PhxShaderCache::setTimeDependent()
{
	UT_String raw;
	evalStringRaw(raw, "cache_path", 0, 0.0f);

	// Check if file contains frame pattern "$F".
	flags().setTimeDep(raw.findString("$F", false, false));
}

void PhxShaderCache::updatePrimitive(const OP_Context &context)
{
	vassert(m_primPacked);

	const fpreal t = context.getTime();

	OP_Options primOptions;
	for (int i = 0; i < getParmList()->getEntries(); ++i) {
		const PRM_Parm &prm = getParm(i);
		primOptions.setOptionFromTemplate(this, prm, *prm.getTemplatePtr(), t);
	}

	const int isTimeDependent = flags().getTimeDep();

	UT_String rawLoadPath;
	evalStringRaw(rawLoadPath, "cache_path", 0, t);
	primOptions.setOptionS("cache_path_raw", rawLoadPath);

	primOptions.setOptionF("current_frame", isTimeDependent ? context.getFloatFrame() : 0.0);

	updatePrimitiveFromOptions(primOptions);
}

#endif // CGR_HAS_AUR
