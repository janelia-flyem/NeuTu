class ZObjsModel:QAbstractItemModel;
class ZSwcObjsModel:ZObjsModel;
class ZPunctaObjsModel:ZObjsModel;
class ZSwcNodeObjsModel:ZObjsModel;
class ZObjsManagerWidget:QWidget;

cmp ZObjsModel::ZObjsItem;
cmp ZObjsItem::ZObjsItem;
cmp ZObjsItem::QVariant;
agg ZSwcObjsModel::ZStackDoc;
agg ZSwcObjsModel::ZSwcTree;
agg ZSwcObjsModel::Swc_Tree_Node;
cmp ZObjsManagerWidget::QTreeView;
agg ZObjsManagerWidget::ZStackDoc;
agg QTreeView::QAbstractItemModel;
cmp ZStackDoc::ZSwcObjsModel;
cmp ZStackDoc::ZSwcNodeObjsModel;
cmp ZStackDoc::ZPunctaObjsModel;
cmp ZStackDoc::ZSwcTree;
cmp ZSwcTree::Swc_Tree_Node;
agg ZSwcNodeObjsModel::Swc_Tree_Node;
agg ZPunctaObjsModel::ZPunctum;
cmp ZStackDoc::ZPunctum;
agg ZPunctaObjsModel::ZStackDoc;
agg ZSwcObjsModel::ZStackDoc;

