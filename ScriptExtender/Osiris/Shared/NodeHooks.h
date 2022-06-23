#pragma once

#include <GameDefinitions/Osiris.h>
#include <unordered_map>
#include <functional>

BEGIN_NS(esv::lua)
class OsirisCallbackManager;
END_NS()

BEGIN_NS(osidbg)
class Debugger;
END_NS()

BEGIN_SE()

struct NodeWrapOptions
{
	bool WrapIsValid;
	bool WrapPushDownTuple;
	bool WrapPushDownTupleDelete;
	bool WrapInsertTuple;
	bool WrapDeleteTuple;
	bool WrapCallQuery;
};

class NodeVMTWrapper
{
public:
	NodeVMTWrapper(NodeVMT * vmt, NodeWrapOptions & options);
	~NodeVMTWrapper();

	bool WrappedIsValid(Node * node, VirtTupleLL * tuple, AdapterRef * adapter);
	void WrappedPushDownTuple(Node * node, VirtTupleLL * tuple, AdapterRef * adapter, EntryPoint which);
	void WrappedPushDownTupleDelete(Node * node, VirtTupleLL * tuple, AdapterRef * adapter, EntryPoint which);
	void WrappedInsertTuple(Node * node, TuplePtrLL * tuple);
	void WrappedDeleteTuple(Node * node, TuplePtrLL * tuple);
	bool WrappedCallQuery(Node * node, OsiArgumentDesc * args);

	static bool s_WrappedIsValid(Node * node, VirtTupleLL * tuple, AdapterRef * adapter);
	static void s_WrappedPushDownTuple(Node * node, VirtTupleLL * tuple, AdapterRef * adapter, EntryPoint which);
	static void s_WrappedPushDownTupleDelete(Node * node, VirtTupleLL * tuple, AdapterRef * adapter, EntryPoint which);
	static void s_WrappedInsertTuple(Node * node, TuplePtrLL * tuple);
	static void s_WrappedDeleteTuple(Node * node, TuplePtrLL * tuple);
	static bool s_WrappedCallQuery(Node * node, OsiArgumentDesc * args);

private:
	NodeVMT * vmt_;
	NodeWrapOptions & options_;
	NodeVMT originalVmt_;
};

class NodeVMTWrappers
{
public:
	NodeVMTWrappers(NodeVMT ** vmts);
	~NodeVMTWrappers();

	bool WrappedIsValid(Node * node, VirtTupleLL * tuple, AdapterRef * adapter);
	void WrappedPushDownTuple(Node * node, VirtTupleLL * tuple, AdapterRef * adapter, EntryPoint which);
	void WrappedPushDownTupleDelete(Node * node, VirtTupleLL * tuple, AdapterRef * adapter, EntryPoint which);
	void WrappedInsertTuple(Node * node, TuplePtrLL * tuple);
	void WrappedDeleteTuple(Node * node, TuplePtrLL * tuple);
	bool WrappedCallQuery(Node * node, OsiArgumentDesc * args);

	osidbg::Debugger* DebuggerAttachment{ nullptr };
	esv::lua::OsirisCallbackManager* OsirisCallbacksAttachment{ nullptr };

	NodeType GetType(Node * node);
	NodeVMTWrapper & GetWrapper(Node * node);

private:
	NodeVMT ** vmts_;
	std::unique_ptr<NodeVMTWrapper> wrappers_[(unsigned)NodeType::Max + 1];
	std::unordered_map<NodeVMT *, NodeType> vmtToTypeMap_;
};

END_SE()
