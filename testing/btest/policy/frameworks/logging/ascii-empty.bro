# @TEST-USE-PROFILES dataseries
# @TEST-EXEC: bro %INPUT
# @TEST-EXEC: btest-diff ssh.log

@load frameworks/logging/plugins/ascii

redef LogAscii::output_to_stdout = F;
redef LogAscii::separator = "|";
redef LogAscii::empty_field = "EMPTY";
redef LogAscii::unset_field = "NOT-SET";
redef LogAscii::header_prefix = "PREFIX<>";

module SSH;

export {
	redef enum Log::ID += { SSH };

	type Log: record {
		t: time;
		id: conn_id; # Will be rolled out into individual columns.
		status: string &optional;
		country: string &default="unknown";
		b: bool &optional;
	} &log;
}

event bro_init()
{
	Log::create_stream(SSH, [$columns=Log]);

    local cid = [$orig_h=1.2.3.4, $orig_p=1234/tcp, $resp_h=2.3.4.5, $resp_p=80/tcp];

	Log::write(SSH, [$t=network_time(), $id=cid, $status="success"]);
	Log::write(SSH, [$t=network_time(), $id=cid, $country="US"]);
	Log::write(SSH, [$t=network_time(), $id=cid, $status="failure", $country="UK"]);
	Log::write(SSH, [$t=network_time(), $id=cid, $country="BR"]);
	Log::write(SSH, [$t=network_time(), $id=cid, $b=T, $status="failure", $country=""]);
	
}

