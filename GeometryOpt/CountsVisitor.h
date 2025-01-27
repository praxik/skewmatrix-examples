//
// Copyright (c) 2008-2009 Skew Matrix Software LLC.
// All rights reserved.
//

#ifndef __COUNTS_VISITOR_H__
#define __COUNTS_VISITOR_H__

#include <osg/NodeVisitor>
#include <set>

class CountsVisitor : public osg::NodeVisitor
{
public:
    CountsVisitor( osg::NodeVisitor::TraversalMode mode = osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN );
    ~CountsVisitor();

    void reset();

    void dump();

    void apply( osg::Node& node );
    void apply( osg::Group& node );
    void apply( osg::LOD& node );
    void apply( osg::PagedLOD& node );
    void apply( osg::Switch& node );
    void apply( osg::Sequence& node );
    void apply( osg::Transform& node );
    void apply( osg::MatrixTransform& node );
    void apply( osg::Geode& node );

    int getVertices() const;
    int getDrawArrays() const;

protected:
    int _depth;
    int _maxDepth;

    int _nodes;
    int _groups;
    int _lods;
    int _pagedLods;
    int _switches;
    int _sequences;
    int _transforms;
    int _matrixTransforms;
    int _dofTransforms;
    int _geodes;
    int _drawables;
    int _geometries;
    int _nullGeometries;
    int _texts;
    int _vertices;
    int _stateSets;
    int _textures;
    int _primitiveSets;
    int _drawArrays;

    int _totalChildren;
    int _slowPathGeometries;

    typedef std::set< osg::ref_ptr<osg::Object> > ObjectSet;
    ObjectSet _uNodes;
    ObjectSet _uGroups;
    ObjectSet _uLods;
    ObjectSet _uPagedLods;
    ObjectSet _uSwitches;
    ObjectSet _uSequences;
    ObjectSet _uTransforms;
    ObjectSet _uMatrixTransforms;
    ObjectSet _uDofTransforms;
    ObjectSet _uGeodes;
    ObjectSet _uDrawables;
    ObjectSet _uGeometries;
    ObjectSet _uTexts;
    ObjectSet _uVertices;
    ObjectSet _uStateSets;
    ObjectSet _uTextures;
    ObjectSet _uPrimitiveSets;
    ObjectSet _uDrawArrays;
};

#endif
