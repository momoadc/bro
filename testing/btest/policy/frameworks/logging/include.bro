# @TEST-USE-PROFILES dataseries
# @TEST-EXEC: bro %INPUT
# @TEST-EXEC: btest-diff ssh.log

module SSH;

export {
	redef enum Log::ID += { SSH };

	type Log: record {
		t: time;
		id: conn_id; # Will be rolled out into individual columns.
		status: string &optional;
		country: string &default="unknown";
	} &log;
}

event bro_init()
{
	Log::create_stream(SSH, [$columns=Log]);

	Log::remove_default_filter(SSH);
	Log::add_filter(SSH, [$name="default", $include=set("t", "id.orig_h")]);

	local cid = [$orig_h=1.2.3.4, $orig_p=1234/tcp, $resp_h=2.3.4.5, $resp_p=80/tcp];

	Log::write(SSH, [$t=network_time(), $id=cid, $status="success"]);
	Log::write(SSH, [$t=network_time(), $id=cid, $status="failure", $country="US"]);
	Log::write(SSH, [$t=network_time(), $id=cid, $status="failure", $country="UK"]);
	Log::write(SSH, [$t=network_time(), $id=cid, $status="success", $country="BR"]);
	Log::write(SSH, [$t=network_time(), $id=cid, $status="failure", $country="MX"]);
	
}

