/*
 * Copyright (c) 2001-2004 International Computer Science Institute
 * See LICENSE file for licensing, conditions, and warranties on use.
 *
 * DO NOT EDIT THIS FILE - IT IS PROGRAMMATICALLY GENERATED
 *
 * Generated by 'clnt-gen'.
 */

#ident "$XORP: xorp/xrl/interfaces/fea_rawpkt6_xif.cc,v 1.3 2004/12/14 18:37:05 atanu Exp $"

#include "fea_rawpkt6_xif.hh"

bool
XrlRawPacket6V0p1Client::send_send_raw(
	const char*	the_tgt,
	const IPv6&	src_address,
	const IPv6&	dst_address,
	const string&	vif_name,
	const uint32_t&	proto,
	const uint32_t&	tclass,
	const uint32_t&	hoplimit,
	const vector<uint8_t>&	hopopts,
	const vector<uint8_t>&	packet,
	const SendRawCB&	cb
)
{
    Xrl x(the_tgt, "raw_packet6/0.1/send_raw");
    x.args().add("src_address", src_address);
    x.args().add("dst_address", dst_address);
    x.args().add("vif_name", vif_name);
    x.args().add("proto", proto);
    x.args().add("tclass", tclass);
    x.args().add("hoplimit", hoplimit);
    x.args().add("hopopts", hopopts);
    x.args().add("packet", packet);
    return _sender->send(x, callback(this, &XrlRawPacket6V0p1Client::unmarshall_send_raw, cb));
}


/* Unmarshall send_raw */
void
XrlRawPacket6V0p1Client::unmarshall_send_raw(
	const XrlError&	e,
	XrlArgs*	a,
	SendRawCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlRawPacket6V0p1Client::send_register_vif_receiver(
	const char*	the_tgt,
	const string&	router_name,
	const string&	if_name,
	const string&	vif_name,
	const uint32_t&	proto,
	const RegisterVifReceiverCB&	cb
)
{
    Xrl x(the_tgt, "raw_packet6/0.1/register_vif_receiver");
    x.args().add("router_name", router_name);
    x.args().add("if_name", if_name);
    x.args().add("vif_name", vif_name);
    x.args().add("proto", proto);
    return _sender->send(x, callback(this, &XrlRawPacket6V0p1Client::unmarshall_register_vif_receiver, cb));
}


/* Unmarshall register_vif_receiver */
void
XrlRawPacket6V0p1Client::unmarshall_register_vif_receiver(
	const XrlError&	e,
	XrlArgs*	a,
	RegisterVifReceiverCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlRawPacket6V0p1Client::send_unregister_vif_receiver(
	const char*	the_tgt,
	const string&	router_name,
	const string&	if_name,
	const string&	vif_name,
	const uint32_t&	proto,
	const UnregisterVifReceiverCB&	cb
)
{
    Xrl x(the_tgt, "raw_packet6/0.1/unregister_vif_receiver");
    x.args().add("router_name", router_name);
    x.args().add("if_name", if_name);
    x.args().add("vif_name", vif_name);
    x.args().add("proto", proto);
    return _sender->send(x, callback(this, &XrlRawPacket6V0p1Client::unmarshall_unregister_vif_receiver, cb));
}


/* Unmarshall unregister_vif_receiver */
void
XrlRawPacket6V0p1Client::unmarshall_unregister_vif_receiver(
	const XrlError&	e,
	XrlArgs*	a,
	UnregisterVifReceiverCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}
