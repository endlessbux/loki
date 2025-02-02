//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file BinaryValue.h
 * @author Ingmar Baumgart
 */

#ifndef __BINARYVALUE_H_
#define __BINARYVALUE_H_

#include "common/OverSimDefs.h"

class BinaryValue : public std::vector<char>, public cObject {
  public:
    static const BinaryValue UNSPECIFIED_VALUE;

    BinaryValue();
    BinaryValue(size_t n);
    BinaryValue(const std::string& str);
    BinaryValue(const std::vector<char>& v);
    BinaryValue(const char *b, const size_t l);
    BinaryValue(const char* cStr);
    BinaryValue(cObject* obj);
    virtual ~BinaryValue() {};

    bool operator<(const BinaryValue& rhs);
    BinaryValue& operator+=(const BinaryValue& rhs);

    friend std::ostream& operator<< (std::ostream& os, const BinaryValue& v);

    virtual void netPack(cCommBuffer *b);
    virtual void netUnpack(cCommBuffer *b);

    void packObject(cObject* obj);
    cObject* unpackObject();

    bool isUnspecified() const;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, BinaryValue& obj) {obj.netPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, BinaryValue& obj) {obj.netUnpack(b);}

#endif
