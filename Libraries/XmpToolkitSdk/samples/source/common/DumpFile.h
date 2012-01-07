// =================================================================================================
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
//
// =================================================================================================

#ifndef XMPQE_DUMPFILE_H
#define XMPQE_DUMPFILE_H

#include "TagTree.h"

class DumpFile {
public:
	static void Scan(std::string filename, TagTree &tagTree);
};

#endif

