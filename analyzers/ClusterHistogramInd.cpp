#ifndef MCMD_CLUSTER_HISTOGRAM_IND_CPP
#define MCMD_CLUSTER_HISTOGRAM_IND_CPP

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2012, David Morse (morse012@umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "ClusterHistogramInd.h"
#include <mcMd/simulation/System.h>
#include <mcMd/simulation/Simulation.h>
#include <simp/species/Species.h>
#include <mcMd/chemistry/Molecule.h>
#include <mcMd/chemistry/Atom.h>
#include <util/misc/FileMaster.h>        
#include <util/archives/Serializable_includes.h>

#include <util/format/Int.h>
#include <util/format/Dbl.h>
#include <util/misc/ioUtil.h>
#include <sstream>

namespace McMd
{

   using namespace Util;

   /// Constructor.
   ClusterHistogramInd::ClusterHistogramInd(System& system) 
    : SystemAnalyzer<System>(system),
      identifier_(system),
      hist_(),
      outputFile_(),
      speciesId_(),
      atomTypeId_(),
      cutoff_(),
      histMin_(),
      histMax_(),
      nSample_(0),
      isInitialized_(false)
   {  setClassName("ClusterHistogramInd"); }

   /// Read parameters from file, and allocate arrays.
   void ClusterHistogramInd::readParameters(std::istream& in) 
   {  
      readInterval(in);
      readOutputFileName(in);
      read<int>(in, "speciesId", speciesId_);
      if (speciesId_ < 0) {
         UTIL_THROW("Negative speciesId");
      }
      if (speciesId_ >= system().simulation().nSpecies()) {
         UTIL_THROW("speciesId > nSpecies");
      }
      Species* speciesPtr;
      speciesPtr = &(system().simulation().species(speciesId_)); 
      isMutable_ = (speciesPtr->isMutable());
      if (isMutable_) {
         read<int>(in, "speciesSubtype", speciesSubtype_);
      }
      read<int>(in, "atomTypeId", atomTypeId_);
      if (atomTypeId_ < 0) {
         UTIL_THROW("Negative atomTypeId");
      }
      if (atomTypeId_ >= system().simulation().nAtomType()) {
         UTIL_THROW("nTypeId >= nAtomType");
      }

      read<double>(in, "cutoff", cutoff_);
      if (cutoff_ < 0) {
         UTIL_THROW("Negative cutoff");
      }

      // Initialize ClusterIdentifier
      identifier_.initialize(speciesId_, atomTypeId_, cutoff_, speciesSubtype_);
      read<int>(in,"histMin", histMin_);
      read<int>(in,"histMax", histMax_);
      hist_.setParam(histMin_, histMax_);
      hist_.clear();
    
      fileMaster().openOutputFile(outputFileName(".dat"), outputFile_);
      isInitialized_ = true;
   }

   /*
   * Load state from an archive.
   */
   void ClusterHistogramInd::loadParameters(Serializable::IArchive& ar)
   {
      // Load interval and outputFileName
      Analyzer::loadParameters(ar);

      loadParameter<int>(ar,"speciesId", speciesId_);
      if (speciesId_ < 0) {
         UTIL_THROW("Negative speciesId");
      }
      if (speciesId_ >= system().simulation().nSpecies()) {
         UTIL_THROW("speciesId > nSpecies");
      }

      Species* speciesPtr;
      speciesPtr = &(system().simulation().species(speciesId_)); 
      isMutable_ = (speciesPtr->isMutable());
      if (isMutable_) {
         loadParameter<int>(ar, "speciesSubtype", speciesSubtype_);
      }

      loadParameter<int>(ar, "atomTypeId", atomTypeId_);
      if (atomTypeId_ < 0) {
         UTIL_THROW("Negative atomTypeId");
      }

      loadParameter<double>(ar, "cutoff", cutoff_);
      if (cutoff_ < 0) {
         UTIL_THROW("Negative cutoff");
      }

      identifier_.initialize(speciesId_, atomTypeId_, cutoff_, speciesSubtype_);
      loadParameter<int>(ar, "histMin", histMin_);
      loadParameter<int>(ar, "histMax", histMax_);
      ar >> hist_;

      ar >> nSample_;

      isInitialized_ = true;
   }

   /*
   * Save state to archive.
   */
   void ClusterHistogramInd::save(Serializable::OArchive& ar)
   {
      Analyzer::save(ar);
      ar & speciesId_;
      ar & atomTypeId_;
      ar & cutoff_;
      ar & histMin_;
      ar & histMax_;
      ar & hist_;
      ar & nSample_;
   }

   /*
   * Clear accumulators.
   */
   void ClusterHistogramInd::setup() 
   {  
      std::cout << "Begininng amin";
      if (!isInitialized_) UTIL_THROW("Object is not initialized");
      hist_.clear();
      nSample_ = 0;
      std::cout << "Begininng amin";
   }

   /* 
   * Evaluate end-to-end vectors of all chains, add to ensemble.
   */
   void ClusterHistogramInd::sample(long iStep) 
   {   
      hist_.clear();
      std::cout << "Begininng amin";
      if (isAtInterval(iStep)) {
         identifier_.identifyClusters();
         for (int i = 0; i < identifier_.nCluster(); i++) {
             hist_.sample(identifier_.cluster(i).size());
         }
         ++nSample_;
      }


      int min = hist_.min();
      int nBin = hist_.nBin();
      for (int i = 0; i < nBin; ++i) {
         outputFile_ << Int(i + min) << "  " 
                     <<  Dbl(double(hist_.data()[i])) << "\n";
      }
   }

   /*
   * Output results to file after simulation is completed.
   */
   void ClusterHistogramInd::output() 
   {  outputFile_.close();
      // Write parameter file
      fileMaster().openOutputFile(outputFileName(".prm"), outputFile_);
      writeParam(outputFile_);
      outputFile_.close();

      // Write histogram output
      fileMaster().openOutputFile(outputFileName(".hist"), outputFile_);
      // hist_.output(outputFile_);
      int min = hist_.min();
      int nBin = hist_.nBin();
      for (int i = 0; i < nBin; ++i) {
         outputFile_ << Int(i + min) << "  " 
                     <<  Dbl(double(hist_.data()[i])/double(nSample_)) << "\n";
      }
      outputFile_.close();
   }

}
#endif 
