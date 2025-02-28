// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <Akri_Facet.hpp>
#include <Akri_BoundingBox.hpp>
#include <Akri_Transformation.hpp>
#include <stk_util/parallel/ParallelComm.hpp>


namespace krino{

//
//--------------------------------------------------------------------------------

static void unpack_vector3d_from_buffer( stk::CommBuffer & b, stk::math::Vector3d & coords )
{
  b.unpack(coords[0]);
  b.unpack(coords[1]);
  b.unpack(coords[2]);
}

static void unpack_vector2d_from_buffer( stk::CommBuffer & b, stk::math::Vector3d & coords )
{
  b.unpack(coords[0]);
  b.unpack(coords[1]);
  coords[2] = 0.;
}

//--------------------------------------------------------------------------------

void
Facet2d::pack_into_buffer(stk::CommBuffer & b) const
{
  for (int n = 0; n < 2; ++n )
  {
    b.pack(myCoords[n][0]);
    b.pack(myCoords[n][1]);
  }
}

void
Facet2d::unpack_facet_data_from_buffer( stk::CommBuffer & b, std::array<stk::math::Vector3d,2> & facetCoords)
{
  unpack_vector2d_from_buffer(b, facetCoords[0]);
  unpack_vector2d_from_buffer(b, facetCoords[1]);
}

void
Facet3d::pack_into_buffer(stk::CommBuffer & b) const
{
  for (int n = 0; n < 3; ++n )
  {
    b.pack(myCoords[n][0]);
    b.pack(myCoords[n][1]);
    b.pack(myCoords[n][2]);
  }
}

void
Facet3d::unpack_facet_data_from_buffer( stk::CommBuffer & b, std::array<stk::math::Vector3d,3> & facetCoords)
{
  unpack_vector3d_from_buffer(b, facetCoords[0]);
  unpack_vector3d_from_buffer(b, facetCoords[1]);
  unpack_vector3d_from_buffer(b, facetCoords[2]);
}

std::ostream & Facet3d::put( std::ostream & os ) const
{
  os << ": facet description: " << std::endl
  << " facet point 0 = ("
  << facet_vertex(0)[0] << ","
  << facet_vertex(0)[1] << ","
  << facet_vertex(0)[2] << ")" << std::endl
  << " facet point 1 = ("
  << facet_vertex(1)[0] << ","
  << facet_vertex(1)[1] << ","
  << facet_vertex(1)[2] << ")" << std::endl
  << " facet point 2 = ("
  << facet_vertex(2)[0] << ","
  << facet_vertex(2)[1] << ","
  << facet_vertex(2)[2] << ")" << std::endl
  << " facet area = " << facet_area() << std::endl;

  return os ;
}

void Facet3d::apply_transformation(const Transformation & transformation)
{
  transformation.apply(myCoords[0]);
  transformation.apply(myCoords[1]);
  transformation.apply(myCoords[2]);
}

void Facet2d::apply_transformation(const Transformation & transformation)
{
  transformation.apply(myCoords[0]);
  transformation.apply(myCoords[1]);
}

std::ostream & Facet2d::put( std::ostream & os ) const
{
  os << ": facet description: " << std::endl
     << " facet point 0 = ("
     << facet_vertex(0)[0] << ","
     << facet_vertex(0)[1] << ")" << std::endl
     << " facet point 1 = ("
     << facet_vertex(1)[0] << ","
     << facet_vertex(1)[1] << ")" << std::endl
     << " facet area = " << facet_area() << std::endl;

  return os ;
}

Facet3d::Facet3d( const stk::math::Vector3d & pt0,
    const stk::math::Vector3d & pt1,
    const stk::math::Vector3d & pt2 )
    : myCoords{pt0, pt1, pt2}
{
}

Facet2d::Facet2d( const stk::math::Vector3d & pt0,
    const stk::math::Vector3d & pt1  )
    : myCoords{pt0, pt1}
{
}

template <typename VecType, size_t N>
bool are_all_components_lo(const VecType & bboxMin, const std::array<const double*,N> & points, const unsigned comp)
{
  for (size_t i=0; i<N; ++i)
    if (points[i][comp] >= bboxMin[comp])
      return false;
  return true;
}

template <typename VecType, size_t N>
bool are_all_components_hi(const VecType & bboxMax, const std::array<const double*,N> & points, const unsigned comp)
{
  for (size_t i=0; i<N; ++i)
    if (points[i][comp] <= bboxMax[comp])
      return false;
  return true;
}

template <size_t N>
bool does_bounding_box_intersect_facet(const BoundingBox & bbox, const std::array<const double*,N> & points, const unsigned ndim)
{
  for (unsigned comp=0; comp<ndim; ++comp)
    if (are_all_components_lo(bbox.get_min(), points, comp) ||
        are_all_components_hi(bbox.get_max(), points, comp))
      return false;
  return true;
}

void Facet3d::insert_into(BoundingBox & bbox) const
{
  bbox.accommodate(myCoords[0]);
  bbox.accommodate(myCoords[1]);
  bbox.accommodate(myCoords[2]);
}

bool Facet3d::does_intersect(const BoundingBox & bbox) const
{
  const std::array<const double*,3> pointData{myCoords[0].data(), myCoords[1].data(), myCoords[2].data()};
  return does_bounding_box_intersect_facet(bbox, pointData, 3);
}

void Facet2d::insert_into(BoundingBox & bbox) const
{
  bbox.accommodate(myCoords[0]);
  bbox.accommodate(myCoords[1]);
}

bool Facet2d::does_intersect(const BoundingBox & bbox) const
{
  const std::array<const double*,2> pointData{myCoords[0].data(), myCoords[1].data()};
  return does_bounding_box_intersect_facet(bbox, pointData, 2);
}

double Facet3d::mean_squared_edge_length() const
{
  return 1./3.*(
      (myCoords[0]-myCoords[1]).length_squared() +
      (myCoords[1]-myCoords[2]).length_squared() +
      (myCoords[2]-myCoords[0]).length_squared());
}

} // namespace krino
