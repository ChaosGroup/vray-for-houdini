//
// Copyright (c) 2015, Chaos Software Ltd
//
// V-Ray For Houdini
//
// ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
//
// Full license text: https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
//

#include "vfh_prm_templates.h"

#include <OP/OP_Node.h>


int VRayForHoudini::Parm::isParmExist(OP_Node &node, const std::string &attrName)
{
	int parmExist = false;

	const PRM_ParmList *parmList = node.getParmList();
	if (parmList) {
		const PRM_Parm *param = parmList->getParmPtr(attrName.c_str());
		if (param) {
			parmExist = true;
		}
	}

	return parmExist;
}


int VRayForHoudini::Parm::isParmSwitcher(OP_Node &node, const int index)
{
	int isSwitcher = false;

	const PRM_ParmList *parmList = node.getParmList();
	if (parmList) {
		const PRM_Parm *param = parmList->getParmPtr(index);
		if (param) {
			isSwitcher = param->getType().isSwitcher();
		}
	}

	return isSwitcher;
}


const PRM_Parm* VRayForHoudini::Parm::getParm(OP_Node &node, const int index)
{
	const PRM_Parm *param = nullptr;

	const PRM_ParmList *parmList = node.getParmList();
	if (parmList) {
		param = parmList->getParmPtr(index);
	}

	return param;
}