/*
 * Copyright (c) 2001-2003 International Computer Science Institute
 * See LICENSE file for licensing, conditions, and warranties on use.
 *
 * DO NOT EDIT THIS FILE - IT IS PROGRAMMATICALLY GENERATED
 *
 * Generated by 'clnt-gen'.
 */

#ident "$XORP$"

#include "redist_transaction6_xif.hh"

bool
XrlRedistTransaction6V0p1Client::send_start_transaction(
	const char*	the_tgt,
	const StartTransactionCB&	cb
)
{
    Xrl x(the_tgt, "redist_transaction6/0.1/start_transaction");
    return _sender->send(x, callback(this, &XrlRedistTransaction6V0p1Client::unmarshall_start_transaction, cb));
}


/* Unmarshall start_transaction */
void
XrlRedistTransaction6V0p1Client::unmarshall_start_transaction(
	const XrlError&	e,
	XrlArgs*	a,
	StartTransactionCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != 1)", (uint32_t)a->size());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    uint32_t tid;
    try {
	a->get("tid", tid);
    } catch (const XrlArgs::XrlAtomNotFound&) {
	XLOG_ERROR("Atom not found");
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &tid);
}

bool
XrlRedistTransaction6V0p1Client::send_commit_transaction(
	const char*	the_tgt,
	const uint32_t&	tid,
	const CommitTransactionCB&	cb
)
{
    Xrl x(the_tgt, "redist_transaction6/0.1/commit_transaction");
    x.args().add("tid", tid);
    return _sender->send(x, callback(this, &XrlRedistTransaction6V0p1Client::unmarshall_commit_transaction, cb));
}


/* Unmarshall commit_transaction */
void
XrlRedistTransaction6V0p1Client::unmarshall_commit_transaction(
	const XrlError&	e,
	XrlArgs*	a,
	CommitTransactionCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != 0)", (uint32_t)a->size());
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlRedistTransaction6V0p1Client::send_abort_transaction(
	const char*	the_tgt,
	const uint32_t&	tid,
	const AbortTransactionCB&	cb
)
{
    Xrl x(the_tgt, "redist_transaction6/0.1/abort_transaction");
    x.args().add("tid", tid);
    return _sender->send(x, callback(this, &XrlRedistTransaction6V0p1Client::unmarshall_abort_transaction, cb));
}


/* Unmarshall abort_transaction */
void
XrlRedistTransaction6V0p1Client::unmarshall_abort_transaction(
	const XrlError&	e,
	XrlArgs*	a,
	AbortTransactionCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != 0)", (uint32_t)a->size());
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlRedistTransaction6V0p1Client::send_add_route(
	const char*	the_tgt,
	const uint32_t&	tid,
	const IPv6Net&	dst,
	const IPv6&	nh,
	const string&	ifname,
	const string&	vifname,
	const uint32_t&	metric,
	const uint32_t&	ad,
	const string&	cookie,
	const AddRouteCB&	cb
)
{
    Xrl x(the_tgt, "redist_transaction6/0.1/add_route");
    x.args().add("tid", tid);
    x.args().add("dst", dst);
    x.args().add("nh", nh);
    x.args().add("ifname", ifname);
    x.args().add("vifname", vifname);
    x.args().add("metric", metric);
    x.args().add("ad", ad);
    x.args().add("cookie", cookie);
    return _sender->send(x, callback(this, &XrlRedistTransaction6V0p1Client::unmarshall_add_route, cb));
}


/* Unmarshall add_route */
void
XrlRedistTransaction6V0p1Client::unmarshall_add_route(
	const XrlError&	e,
	XrlArgs*	a,
	AddRouteCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != 0)", (uint32_t)a->size());
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlRedistTransaction6V0p1Client::send_delete_route(
	const char*	the_tgt,
	const uint32_t&	tid,
	const IPv6Net&	network,
	const string&	cookie,
	const DeleteRouteCB&	cb
)
{
    Xrl x(the_tgt, "redist_transaction6/0.1/delete_route");
    x.args().add("tid", tid);
    x.args().add("network", network);
    x.args().add("cookie", cookie);
    return _sender->send(x, callback(this, &XrlRedistTransaction6V0p1Client::unmarshall_delete_route, cb));
}


/* Unmarshall delete_route */
void
XrlRedistTransaction6V0p1Client::unmarshall_delete_route(
	const XrlError&	e,
	XrlArgs*	a,
	DeleteRouteCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != 0)", (uint32_t)a->size());
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}
