// @(#)root/tree:$Name:  $:$Id: TMethodBrowsable.cxx,v 1.4 2004/11/05 17:01:58 brun Exp $
// Author: Axel Naumann   14/10/2004

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TMethodBrowsable                                                     //
//                                                                      //
// A helper object to browse methods (see                               //
// TBranchElement::GetBrowsableMethods)                                 //
//////////////////////////////////////////////////////////////////////////

#include "TMethodBrowsable.h"
#include "TBranchElement.h"
#include "TMethod.h"
#include "TBrowser.h"
#include "TTree.h"
#include "TPad.h"
#include "TClass.h"
#include "TBaseClass.h"
#include "TDataMember.h"

ClassImp(TMethodBrowsable);

//______________________________________________________________________________
TMethodBrowsable::TMethodBrowsable(TBranchElement* be, TMethod* m,
                                   TMethodBrowsable* parent /* =0 */):
      fBranchElement(be), fParent(parent), fMethod(m), 
      fReturnClass(0), fReturnLeafs(0), fReturnIsPointer(kFALSE) {
// standard constructor.
// Links a TBranchElement with a TMethod, allowing the TBrowser to
// Browser simple methods.
//
// The c'tor sets the name for a method "Class::Method(params) const"
// to "Method(params)", title to TMethod::GetPrototype
   TString name(m->GetName());
   name+="()";
   if (name.EndsWith(" const")) name.Remove(name.Length()-6);
   SetName(name);

   name=m->GetPrototype();
   if (m->GetCommentString() && strlen(m->GetCommentString()))
      name.Append(" // ").Append(m->GetCommentString());
   SetTitle(name);

   TString plainReturnType(m->GetReturnTypeName());
   if (plainReturnType.EndsWith("*")) {
      fReturnIsPointer=kTRUE;
      plainReturnType.Remove(plainReturnType.Length()-1);
      plainReturnType.Strip();
      if(plainReturnType.BeginsWith("const")) {
        plainReturnType.Remove(0,5);
        plainReturnType.Strip();
      }   
   }
   fReturnClass=gROOT->GetClass(plainReturnType);
}

//______________________________________________________________________________
void TMethodBrowsable::Browse(TBrowser *b) {
// Calls TTree::Draw on the method if return type is not a class;
// otherwise expands returned object's "folder"

   if (!fReturnClass) {
      TString name;
      GetScope(name);
      fBranchElement->GetTree()->Draw(name, "", b ? b->GetDrawOption() : "");
      if (gPad) gPad->Update();
   } else {
      if (!fReturnLeafs)
         fReturnLeafs=GetMethodBrowsables(fBranchElement, fReturnClass, this);
      if (fReturnLeafs)
         fReturnLeafs->Browse(b);
   }
}

//______________________________________________________________________________
TList* TMethodBrowsable::GetMethodBrowsables(TBranchElement* be, TClass* cl,
                                             TMethodBrowsable* parent /* =0 */) {
// Given a class, this methods returns a list of TMethodBrowsables
// for the class and its base classes.
// This list has to be deleted by the caller!

   if (!cl) return 0;

   TList allClasses;
   allClasses.Add(cl);
   
   for(TObjLink* lnk=allClasses.FirstLink();
       lnk; lnk=lnk->Next()){
      cl=(TClass*)lnk->GetObject();
      TList* bases=cl->GetListOfBases();
      TBaseClass* base;
      TIter iB(bases);
      while ((base=(TBaseClass*)iB())) {
         TClass* bc=base->GetClassPointer();
         if (bc) allClasses.Add(bc);
      }
   }

   TList allMethods;
   TIter iC(&allClasses);
   while ((cl=(TClass*)iC())) {
      TList* methods=cl->GetListOfMethods();
      if (!methods) continue;
      TMethod* method=0;
      TIter iM(methods);
      while ((method=(TMethod*)iM()))
         if (method && !allMethods.FindObject(method->GetName()))
             allMethods.Add(method);
   }

   TIter iM(&allMethods);
   TMethod* m=0;
   TList* browsableMethods=new TList();
   browsableMethods->SetOwner();
   while ((m=(TMethod*)iM()))
      if (TMethodBrowsable::IsMethodBrowsable(m))
         browsableMethods->Add(new TMethodBrowsable(be, m, parent));

   return browsableMethods;
}

//______________________________________________________________________________
void TMethodBrowsable::GetScope(TString & scope) {
// Returns the full name for TTree::Draw to draw *this.
// Recursively appends, starting form the top TBranchElement,
// all method names with proper reference operators (->, .)
// depending on fReturnIsPointer.

   if (fParent)
      fParent->GetScope(scope);
   else {
      scope=fBranchElement->GetName();
      scope+=".";
      TBranch* mother=fBranchElement;
      while (mother != mother->GetMother() && (mother=mother->GetMother()))
         scope.Prepend(".").Prepend(mother->GetName());
   }
   scope+=GetName();
   if (fReturnClass) // otherwise we're a leaf, and we are the one drawn
      if (fReturnIsPointer)
         scope+="->";
      else scope+=".";
}

//______________________________________________________________________________
Bool_t TMethodBrowsable::IsMethodBrowsable(TMethod* m) {
// A TMethod is browsable if it is const, public and not pure virtual,
// if does not have any parameter without default value, and if it has 
// a (non-void) return value.
// A method called *, Get*, or get* will not be browsable if there is a 
// persistent data member called f*, _*, or m*, as data member access is 
// faster than method access. Examples: if one of fX, _X, or mX is a 
// persistent data member, the methods GetX(), getX(), and X() will not 
// be browsable.

   if (m->GetNargs()-m->GetNargsOpt()==0
       && (m->Property() & kIsConstant 
           & ~kIsPrivate & ~kIsProtected & ~kIsPureVirtual )
       && m->GetReturnTypeName()
       && strcmp("void",m->GetReturnTypeName())
       && !strstr(m->GetName(),"DeclFile")
       && !strstr(m->GetName(),"ImplFile")
       && strcmp(m->GetName(),"IsA")
       && strcmp(m->GetName(),"Class")
       && strcmp(m->GetName(),"CanBypassStreamer")
       && strcmp(m->GetName(),"Class_Name")
       && strcmp(m->GetName(),"ClassName")
       && strcmp(m->GetName(),"Clone")
       && strcmp(m->GetName(),"DrawClone")
       && strcmp(m->GetName(),"GetName")
       && strcmp(m->GetName(),"GetDrawOption")
       && strcmp(m->GetName(),"GetIconName")
       && strcmp(m->GetName(),"GetOption")
       && strcmp(m->GetName(),"GetTitle")
       && strcmp(m->GetName(),"GetUniqueID")
       && strcmp(m->GetName(),"Hash")
       && strcmp(m->GetName(),"IsFolder")
       && strcmp(m->GetName(),"IsOnHeap")
       && strcmp(m->GetName(),"IsSortable")
       && strcmp(m->GetName(),"IsZombie")) {
         // look for matching data member
         TClass* cl=m->GetClass();
         if (!cl) return kTRUE;
         TList* members=cl->GetListOfDataMembers();
         if (!members) return kTRUE;
         const char* baseName=m->GetName();
         if (!strncmp(m->GetName(), "Get", 3) ||
             !strncmp(m->GetName(), "get", 3))
            baseName+=3;
         if (!baseName[0]) return kTRUE;

         TObject* mem=0;
         const char* arrMemberNames[3]={"f%s","_%s","m%s"};
         for (Int_t i=0; !mem && i<3; i++)
            mem=members->FindObject(Form(arrMemberNames[i],baseName));
         return (!mem ||! ((TDataMember*)mem)->IsPersistent());
   };
   return kFALSE;
}
