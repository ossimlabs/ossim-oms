//-----------------------------------------------------------------------------
// File:  KeywordlistIterator.cpp
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Wrapper for ossimKeywordlistIterator.
//
//-----------------------------------------------------------------------------
// $Id: KeywordlistIterator.cpp 22121 2013-01-26 16:34:32Z dburken $

#include <oms/KeywordlistIterator.h>
#include <oms/Keywordlist.h>
#include <oms/StringPair.h>
#include <ossim/base/ossimKeywordlist.h>

oms::KeywordlistIterator::KeywordlistIterator()
   :
   m_kwl(0),
   m_iter()
{
}

oms::KeywordlistIterator::KeywordlistIterator(const oms::KeywordlistIterator& iter)
   :
   m_kwl(iter.m_kwl),
   m_iter(iter.m_iter)
{
}

oms::KeywordlistIterator::KeywordlistIterator(oms::Keywordlist* kwl)
   :
   m_kwl(kwl),
   m_iter()
{
   if ( m_kwl && m_kwl->valid() )
   {
      m_iter = m_kwl->getKeywordlist()->getMap().begin();
   }
}

oms::KeywordlistIterator::~KeywordlistIterator()
{
}

const oms::KeywordlistIterator& oms::KeywordlistIterator::operator=(
   const oms::KeywordlistIterator& iter )
{
   if ( this != &iter )
   {
      m_kwl         = iter.m_kwl;
      m_iter        = iter.m_iter;
   }
   return *this;
}

void oms::KeywordlistIterator::initialize(oms::Keywordlist* kwl)
{
   m_kwl = kwl;
   if ( m_kwl && m_kwl->valid() )
   {
      m_iter = m_kwl->getKeywordlist()->getMap().begin();
   }
}

void oms::KeywordlistIterator::reset()
{
   if ( m_kwl && m_kwl->valid() )
   {
      m_iter = m_kwl->getKeywordlist()->getMap().begin();
   }
}

bool oms::KeywordlistIterator::end() const
{
   bool result = false;
   if ( m_kwl  && m_kwl->valid() )
   {
      result = (m_iter == m_kwl->getKeywordlist()->getMap().end());
   }
   return result;
}

void oms::KeywordlistIterator::next()
{
   if ( !end() )
   {
      ++m_iter;
   }
}

std::string oms::KeywordlistIterator::getKey() const
{
   return (*m_iter).first;
}

std::string oms::KeywordlistIterator::getValue() const
{
   return (*m_iter).second;
}

void oms::KeywordlistIterator::getKeyValue( oms::StringPair* pair ) const
{
   if ( pair )
   {
      pair->setKey( (*m_iter).first );
      pair->setValue( (*m_iter).second );
   }
}
