//
// Copyright (c) 2015-2018, Chaos Software Ltd
//
// V-Ray For Houdini
//
// ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
//
// Full license text: https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
//

#ifndef VRAY_FOR_HOUDINI_VRAY_H
#define VRAY_FOR_HOUDINI_VRAY_H

#include <QMap>
#include <QSet>
#include <QStringBuilder>

#include <SYS/SYS_Types.h>

#include <vraysdk.hpp>

#include <utils.h>
#include <quaternion.h>
#include <spectrum.h>
#include <table.h>
#include <trimesh.h>
#include <pixelbuffer.h>
#include <pstream.h>
#include <systemstuff.h>
#include <misc.h>
#include <ipctools.h>
#include <uni.h>
#include <bmpbuffer.h>
#include <vutils_memcpy.h>
#include <mesh_file.h>
#include <mesh_objects_info.h>
#include <mesh_sets_info.h>
#include <hash_set.h>

// For HDK to work properly
#ifdef INT64
	#undef INT64
#endif

#undef itoa

FORCEINLINE VRay::Transform toAppSdkTm(const VUtils::TraceTransform &tm)
{
	return VRay::Transform(
			VRay::Matrix(
				VRay::Vector(tm.m.f[0].x, tm.m.f[0].y, tm.m.f[0].z),
				VRay::Vector(tm.m.f[1].x, tm.m.f[1].y, tm.m.f[1].z),
				VRay::Vector(tm.m.f[2].x, tm.m.f[2].y, tm.m.f[2].z)
			),
			VRay::Vector(tm.offs.x, tm.offs.y, tm.offs.z));
}

FORCEINLINE VUtils::TraceTransform toVutilsTm(const VRay::Transform &tm)
{
	return VUtils::TraceTransform(
			VUtils::Matrix(
				VUtils::Vector(tm.matrix.v0.x, tm.matrix.v0.y, tm.matrix.v0.z),
				VUtils::Vector(tm.matrix.v1.x, tm.matrix.v1.y, tm.matrix.v1.z),
				VUtils::Vector(tm.matrix.v2.x, tm.matrix.v2.y, tm.matrix.v2.z)
			),
			VUtils::Vector(tm.offset.x, tm.offset.y, tm.offset.z));
}

FORCEINLINE VRay::VUtils::TraceTransform toVRayVutilsTm(const VRay::Transform &tm)
{
	VRay::VUtils::TraceTransform ttm(1.0f);
	ttm.m = tm.matrix;
	ttm.offs = tm.offset;

	return ttm;
}

#define SL(x) QStringLiteral(x)

#endif // VRAY_FOR_HOUDINI_VRAY_H
