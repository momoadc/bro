// See the file "COPYING" in the main distribution directory for copyright.

#include <string>
#include <fcntl.h>

#include "Extract.h"
#include "util.h"
#include "Event.h"
#include "file_analysis/Manager.h"

using namespace file_analysis;

Extract::Extract(RecordVal* args, File* file, const string& arg_filename,
                 uint64_t arg_limit)
    : file_analysis::Analyzer(file_mgr->GetComponentTag("EXTRACT"), args, file),
      filename(arg_filename), limit(arg_limit), depth(0)
	{
	fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0666);

	if ( fd < 0 )
		{
		fd = 0;
		char buf[128];
		bro_strerror_r(errno, buf, sizeof(buf));
		reporter->Error("cannot open %s: %s", filename.c_str(), buf);
		}
	}

Extract::~Extract()
	{
	if ( fd )
		safe_close(fd);
	}

static IntrusivePtr<Val> get_extract_field_val(RecordVal* args, const char* name)
	{
	auto rval = args->Lookup(name);

	if ( ! rval )
		reporter->Error("File extraction analyzer missing arg field: %s", name);

	return rval;
	}

file_analysis::Analyzer* Extract::Instantiate(RecordVal* args, File* file)
	{
	auto fname = get_extract_field_val(args, "extract_filename");
	auto limit = get_extract_field_val(args, "extract_limit");

	if ( ! fname || ! limit )
		return 0;

	return new Extract(args, file, fname->AsString()->CheckString(),
	                   limit->AsCount());
	}

static bool check_limit_exceeded(uint64_t lim, uint64_t depth, uint64_t len, uint64_t* n)
	{
	if ( lim == 0 )
		{
		*n = len;
		return false;
		}

	if ( depth >= lim )
		{
		*n = 0;
		return true;
		}
	else if ( depth + len > lim )
		{
		*n = lim - depth;
		return true;
		}
	else
		{
		*n = len;
		}

	return false;
	}

bool Extract::DeliverStream(const u_char* data, uint64_t len)
	{
	if ( ! fd )
		return false;

	uint64_t towrite = 0;
	bool limit_exceeded = check_limit_exceeded(limit, depth, len, &towrite);

	if ( limit_exceeded && file_extraction_limit )
		{
		File* f = GetFile();
		f->FileEvent(file_extraction_limit, {
			IntrusivePtr{NewRef{}, f->GetVal()},
			IntrusivePtr{NewRef{}, Args()},
			IntrusivePtr{AdoptRef{}, val_mgr->GetCount(limit)},
			IntrusivePtr{AdoptRef{}, val_mgr->GetCount(len)}
		});

		// Limit may have been modified by a BIF, re-check it.
		limit_exceeded = check_limit_exceeded(limit, depth, len, &towrite);
		}

	if ( towrite > 0 )
		{
		safe_write(fd, reinterpret_cast<const char*>(data), towrite);
		depth += towrite;
		}

	return ( ! limit_exceeded );
	}

bool Extract::Undelivered(uint64_t offset, uint64_t len)
	{
	if ( depth == offset )
		{
		char* tmp = new char[len]();
		safe_write(fd, tmp, len);
		delete [] tmp;
		depth += len;
		}

	return true;
	}
