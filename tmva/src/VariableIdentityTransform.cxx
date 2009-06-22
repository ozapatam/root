// @(#)root/tmva $Id$
// Author: Andreas Hoecker, Joerg Stelzer, Helge Voss

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Class  : VariableIdentityTransform                                             *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation (see header for description)                               *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Andreas Hoecker <Andreas.Hocker@cern.ch> - CERN, Switzerland              *
 *      Joerg Stelzer   <Joerg.Stelzer@cern.ch>  - CERN, Switzerland              *
 *      Helge Voss      <Helge.Voss@cern.ch>     - MPI-K Heidelberg, Germany      *
 *                                                                                *
 * Copyright (c) 2005:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/


#include <iostream>

#include "TMVA/VariableIdentityTransform.h"

#ifndef ROOT_TMVA_MsgLogger
#include "TMVA/MsgLogger.h"
#endif

ClassImp(TMVA::VariableIdentityTransform)

//_______________________________________________________________________
   TMVA::VariableIdentityTransform::VariableIdentityTransform( DataSetInfo& dsi )
      : VariableTransformBase( dsi, Types::kIdentity, "Id" )
{
   // constructor
}

//_______________________________________________________________________
void TMVA::VariableIdentityTransform::Initialize()
{
}

//_______________________________________________________________________
Bool_t TMVA::VariableIdentityTransform::PrepareTransformation( const std::vector<Event*>& events)
{
   // the identity does not need to be prepared, only calculate the normalization
   Initialize();

   if (!IsEnabled() || IsCreated()) return kTRUE;

   log() << kINFO << "Preparing the Identity transformation..." << Endl;

   SetNVariables(events[0]->GetNVariables());

   SetCreated( kTRUE );

   return kTRUE;
}

//_______________________________________________________________________
void TMVA::VariableIdentityTransform::AttachXMLTo(void* /*parent*/) {
   log() << kFATAL << "Please implement writing of transformation as XML" << Endl;
}

//_______________________________________________________________________
void TMVA::VariableIdentityTransform::ReadFromXML( void* /*trfnode*/ ) {
   return;
}

//_______________________________________________________________________
const TMVA::Event* TMVA::VariableIdentityTransform::Transform(const TMVA::Event* const ev, Int_t /*cls*/) const 
{
   return ev;
}

//_______________________________________________________________________
void TMVA::VariableIdentityTransform::MakeFunction(std::ostream& fout, const TString& fncName, Int_t /*part*/, UInt_t trCounter, Int_t /*cls*/ ) 
{
   fout << "inline void " << fncName << "::InitTransform_Identity_" << trCounter << "() {}" << std::endl;
   fout << std::endl;
   fout << "inline void " << fncName << "::Transform_Identity_" << trCounter << "(const std::vector<double> &, int) const {}" << std::endl;
}
