#
# Copyright (c) 2015, Chaos Software Ltd
#
# V-Ray For Houdini
#
# ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
#
# Full license text: https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
#

import hou

# tests the node's type, or if a node would error out.
def _validVrayRop(node):
    try:
        result = node.type().name() == 'vray_renderer'
    except:
        result = ''
    
    return result
    

def _createVrayRop():
    vrayRopNode = hou.node('/out').createNode('vray_renderer')
    
    return vrayRopNode


def _getVrayRop():
    vrayRopPath = hou.session.curVrayRopPath if hasattr(hou.session, 'curVrayRopPath') else ''
    
    # safety incase vrayRopPath gets overwritten with a different value type
    if type(vrayRopPath).__name__ != 'str':
        vrayRopPath = ''
    
    vrayRopType = hou.nodeType(hou.ropNodeTypeCategory(), "vray_renderer")
    vrayRopNodes = vrayRopType.instances()
    vrayRopSelection = [i for i in vrayRopNodes if i.isSelected()]
    
    if vrayRopSelection:
        vrayRop = vrayRopSelection[0]
    else:
        vrayRop = hou.node(vrayRopPath)
        if not _validVrayRop(vrayRop):
            if vrayRopNodes:
                vrayRop = vrayRopNodes[0]
            else:
                vrayRop = _createVrayRop()
                
    hou.session.curVrayRopPath = vrayRop.path()
    
    return vrayRop


def render():
    vrayRopNode = _getVrayRop()
    vrayRopNode.parm('execute').pressButton()


def render_rt():
    vrayRopNode = _getVrayRop()
    vrayRopNode.parm('render_rt').pressButton()
