//
// Copyright (c) 2015-2018, Chaos Software Ltd
//
// V-Ray For Houdini
//
// ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
//
// Full license text: https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
//

#ifndef VRAY_FOR_HOUDINI_VOP_NODE_MTLMULTI_H
#define VRAY_FOR_HOUDINI_VOP_NODE_MTLMULTI_H

#include "vop_node_base.h"

namespace VRayForHoudini {
namespace VOP {

class MtlMulti :
	public NodeBase
{
public:
	MtlMulti(OP_Network *parent, const char *name, OP_Operator *entry)
		: NodeBase(parent, name, entry) {}
	virtual ~MtlMulti() VRAY_DTOR_OVERRIDE {}

	// From OP::VRayNode
	PluginResult asPluginDesc(Attrs::PluginDesc &pluginDesc, VRayExporter &exporter) VRAY_OVERRIDE;

protected:
	void setPluginType() VRAY_OVERRIDE;

	// From VOP_Node
public:
	const char *inputLabel(unsigned idx) const VRAY_OVERRIDE;
	unsigned getNumVisibleInputs() const VRAY_OVERRIDE;
	unsigned orderedInputs() const VRAY_OVERRIDE;

protected:
	int getInputFromName(const UT_String &in) const VRAY_OVERRIDE;
	void getInputNameSubclass(UT_String &in, int idx) const VRAY_OVERRIDE;
	int getInputFromNameSubclass(const UT_String &in) const VRAY_OVERRIDE;
	void getInputTypeInfoSubclass(VOP_TypeInfo &type_info, int idx) VRAY_OVERRIDE;
	void getAllowedInputTypeInfosSubclass(unsigned idx, VOP_VopTypeInfoArray &type_infos) VRAY_OVERRIDE;
	void getAllowedInputTypesSubclass(unsigned idx, VOP_VopTypeArray &voptypes) VRAY_OVERRIDE;
	bool willAutoconvertInputType(int input_idx) VRAY_OVERRIDE;

private:
	int customInputsCount() const;
};

} // namespace VOP
} // namespace VRayForHoudini

#endif // VRAY_FOR_HOUDINI_VOP_NODE_MTLMULTI_H
