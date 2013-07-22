////**********************************************************************
////**********************************************************************
////
////  RANDOM FORESTS FOR SURVIVAL, REGRESSION, AND CLASSIFICATION (RF-SRC)
////  Version 1.3
////
////  Copyright 2012, University of Miami
////
////  This program is free software; you can redistribute it and/or
////  modify it under the terms of the GNU General Public License
////  as published by the Free Software Foundation; either version 2
////  of the License, or (at your option) any later version.
////
////  This program is distributed in the hope that it will be useful,
////  but WITHOUT ANY WARRANTY; without even the implied warranty of
////  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
////  GNU General Public License for more details.
////
////  You should have received a copy of the GNU General Public
////  License along with this program; if not, write to the Free
////  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
////  Boston, MA  02110-1301, USA.
////
////  ----------------------------------------------------------------
////  Project Partially Funded By: 
////  ----------------------------------------------------------------
////  Dr. Ishwaran's work was funded in part by DMS grant 1148991 from the
////  National Science Foundation and grant R01 CA163739 from the National
////  Cancer Institute.
////
////  Dr. Kogalur's work was funded in part by grant R01 CA163739 from the 
////  National Cancer Institute.
////  ----------------------------------------------------------------
////  Written by:
////  ----------------------------------------------------------------
////    Hemant Ishwaran, Ph.D.
////    Director of Statistical Methodology
////    Professor, Division of Biostatistics
////    Clinical Research Building, Room 1058
////    1120 NW 14th Street
////    University of Miami, Miami FL 33136
////
////    email:  hemant.ishwaran@gmail.com
////    URL:    http://web.ccs.miami.edu/~hishwaran
////    --------------------------------------------------------------
////    Udaya B. Kogalur, Ph.D.
////    Adjunct Staff
////    Dept of Quantitative Health Sciences
////    Cleveland Clinic Foundation
////    
////    Kogalur & Company, Inc.
////    5425 Nestleway Drive, Suite L1
////    Clemmons, NC 27012
////
////    email:  commerce@kogalur.com
////    URL:    http://www.kogalur.com
////    --------------------------------------------------------------
////
////**********************************************************************
////**********************************************************************


#include        "global.h"
#include        "extern.h"
#include         "trace.h"
#include        "nrutil.h"
#include     "factorOps.h"
#include     "splitUtil.h"
#include    "regression.h"
#include     "splitRegr.h"
char regressionSplit (uint    treeID, 
                      Node   *parent, 
                      uint   *repMembrIndx,
                      uint    repMembrSize,
                      uint   *allMembrIndx,
                      uint    allMembrSize,
                      uint   *splitParameterMax, 
                      double *splitValueMaxCont, 
                      uint   *splitValueMaxFactSize, 
                      uint  **splitValueMaxFactPtr,
                      double *splitStatistic) {
  uint    *randomCovariateIndex;
  double **permissibleSplit;
  uint    *permissibleSplitSize;
  uint   **repMembrIndxx;
  uint priorMembrIter, currentMembrIter;
  uint leftSizeIter, rghtSizeIter;
  uint leftSize, rghtSize;
  char *localSplitIndicator;
  double delta, deltaMax;
  double sumLeft, sumRght, sumRghtSave, sumLeftSqr, sumRghtSqr;
  uint splitLength;
  void *permissibleSplitPtr;
  char factorFlag;
  uint mwcpSizeAbsolute;
  char deterministicSplitFlag;
  char result;
  uint i, j, k;
  mwcpSizeAbsolute       = 0;  
  sumLeft = sumRght      = 0;  
  leftSizeIter           = 0;  
  rghtSizeIter           = 0;  
  *splitParameterMax     = 0;
  *splitValueMaxFactSize = 0;
  *splitValueMaxFactPtr  = NULL;
  *splitValueMaxCont     = NA_REAL;
  deltaMax               = NA_REAL;
  if (repMembrSize >= (2 * RF_minimumNodeSize)) {
    result = TRUE;
  }
  else {
    result = FALSE;
  }
  if (result) {
    if (RF_maximumNodeDepth < 0) {
      result = TRUE;
    }
    else {
      if (parent -> depth < (uint) RF_maximumNodeDepth) {
        result = TRUE;
      }
      else {
        result = FALSE;
      }
    }
  }
  if (result) {
    result = getVariance(repMembrSize, repMembrIndx, RF_response[treeID][1], NULL, NULL);
  }
  if (result) {
    stackSplitIndicator(repMembrSize, & localSplitIndicator);
    uint actualCovariateCount = stackAndSelectRandomCovariates(treeID,
                                                               parent, 
                                                               repMembrIndx, 
                                                               repMembrSize, 
                                                               & randomCovariateIndex, 
                                                               & permissibleSplit, 
                                                               & permissibleSplitSize,
                                                               & repMembrIndxx);
    sumRghtSave = 0.0;
    for (j = 1; j <= repMembrSize; j++) {
      sumRghtSave += RF_response[treeID][1][repMembrIndx[j]];
    }
    for (i = 1; i <= actualCovariateCount; i++) {
      leftSize = 0;
      priorMembrIter = 0;
      splitLength = stackAndConstructSplitVector(treeID,
                                                 repMembrSize,
                                                 randomCovariateIndex[i], 
                                                 permissibleSplit[i], 
                                                 permissibleSplitSize[i],
                                                 & factorFlag,
                                                 & deterministicSplitFlag,
                                                 & mwcpSizeAbsolute,
                                                 & permissibleSplitPtr);
      if (factorFlag == FALSE) {
        for (j = 1; j <= repMembrSize; j++) {
          localSplitIndicator[j] = RIGHT;
        }
        sumRght      = sumRghtSave;
        sumLeft      = 0.0;
        leftSizeIter = 0;
        rghtSizeIter = repMembrSize;
      }
      for (j = 1; j < splitLength; j++) {
        if (factorFlag == TRUE) {
          priorMembrIter = 0;
          leftSize = 0;
        }
        virtuallySplitNodeNew(treeID,
                              factorFlag,
                              mwcpSizeAbsolute,
                              randomCovariateIndex[i],
                              repMembrIndx,
                              repMembrIndxx[i],
                              repMembrSize,
                              permissibleSplitPtr,
                              j,
                              localSplitIndicator,
                              & leftSize,
                              priorMembrIter,
                              & currentMembrIter);
        rghtSize = repMembrSize - leftSize;
        if ((leftSize  >= (RF_minimumNodeSize)) && (rghtSize  >= (RF_minimumNodeSize))) {
          if (factorFlag == TRUE) {
            sumLeft = sumRght = 0.0;
            for (k = 1; k <= repMembrSize; k++) {
              if (localSplitIndicator[k] == LEFT) {
                sumLeft += RF_response[treeID][1][repMembrIndx[k]];
              }
              else {
                sumRght += RF_response[treeID][1][repMembrIndx[k]];
              }
            }
          }
          else {
            for (k = leftSizeIter + 1; k < currentMembrIter; k++) {
              sumLeft += RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]];
              sumRght -= RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]];
            }
            rghtSizeIter = rghtSizeIter - (currentMembrIter - (leftSizeIter + 1));
            leftSizeIter = currentMembrIter - 1; 
          }
          sumLeftSqr = pow (sumLeft, 2.0) / leftSize;
          sumRghtSqr = pow (sumRght, 2.0) / rghtSize;
          delta = sumLeftSqr + sumRghtSqr;
          updateMaximumSplit(delta,
                             randomCovariateIndex[i],
                             j,
                             factorFlag,
                             mwcpSizeAbsolute,
                             & deltaMax,
                             splitParameterMax,
                             splitValueMaxCont,
                             splitValueMaxFactSize,
                             splitValueMaxFactPtr,
                             permissibleSplitPtr);
        }  
        if (factorFlag == FALSE) {
          if (rghtSize  < RF_minimumNodeSize) {
            j = splitLength;
          }
          else {
            priorMembrIter = currentMembrIter - 1;
          }
        }
      }  
      unstackSplitVector(treeID,
                         permissibleSplitSize[i],
                         splitLength,
                         factorFlag,
                         deterministicSplitFlag,
                         mwcpSizeAbsolute,
                         permissibleSplitPtr);
    }  
    unstackRandomCovariates(treeID,
                            repMembrSize, 
                            randomCovariateIndex, 
                            actualCovariateCount,
                            permissibleSplit, 
                            permissibleSplitSize,
                            repMembrIndxx);
    unstackSplitIndicator(repMembrSize, localSplitIndicator);
  }  
  result = summarizeSplitResult(*splitParameterMax, 
                                *splitValueMaxCont,
                                *splitValueMaxFactSize,
                                *splitValueMaxFactPtr,
                                 splitStatistic,
                                 deltaMax);
  return result;
}
char regressionUwghtSplit (uint    treeID, 
                           Node   *parent, 
                           uint   *repMembrIndx,
                           uint    repMembrSize,
                           uint   *allMembrIndx,
                           uint    allMembrSize,
                           uint   *splitParameterMax, 
                           double *splitValueMaxCont, 
                           uint   *splitValueMaxFactSize, 
                           uint  **splitValueMaxFactPtr,
                           double *splitStatistic) {
  uint    *randomCovariateIndex;
  double **permissibleSplit;
  uint    *permissibleSplitSize;
  uint   **repMembrIndxx;
  uint priorMembrIter, currentMembrIter;
  uint leftSizeIter, rghtSizeIter;
  uint leftSize, rghtSize;
  char *localSplitIndicator;
  double delta, deltaMax;
  double sumLeft, sumRght, sumRghtSave, sumRghtSqrSave, sumLeftSqr, sumRghtSqr;
  double leftTemp, rghtTemp, leftTempSqr, rghtTempSqr;
  uint splitLength;
  void *permissibleSplitPtr;
  char factorFlag;
  uint mwcpSizeAbsolute;
  char deterministicSplitFlag;
  char result;
  uint i, j, k;
  mwcpSizeAbsolute       = 0;  
  sumLeft = sumRght      = 0;  
  sumLeftSqr             = 0;  
  sumRghtSqr             = 0;  
  leftSizeIter           = 0;  
  rghtSizeIter           = 0;  
  *splitParameterMax     = 0;
  *splitValueMaxFactSize = 0;
  *splitValueMaxFactPtr  = NULL;
  *splitValueMaxCont     = NA_REAL;
  deltaMax               = NA_REAL;
  if (repMembrSize >= (2 * RF_minimumNodeSize)) {
    result = TRUE;
  }
  else {
    result = FALSE;
  }
  if (result) {
    if (RF_maximumNodeDepth < 0) {
      result = TRUE;
    }
    else {
      if (parent -> depth < (uint) RF_maximumNodeDepth) {
        result = TRUE;
      }
      else {
        result = FALSE;
      }
    }
  }
  if (result) {
    result = getVariance(repMembrSize, repMembrIndx, RF_response[treeID][1], NULL, NULL);
  }
  if (result) {
    stackSplitIndicator(repMembrSize, & localSplitIndicator);
    uint actualCovariateCount = stackAndSelectRandomCovariates(treeID,
                                                               parent, 
                                                               repMembrIndx, 
                                                               repMembrSize, 
                                                               & randomCovariateIndex, 
                                                               & permissibleSplit, 
                                                               & permissibleSplitSize,
                                                               & repMembrIndxx);
    sumRghtSave = sumRghtSqrSave = 0.0;
    for (j = 1; j <= repMembrSize; j++) {
      sumRghtSave += RF_response[treeID][1][repMembrIndx[j]];
      sumRghtSqrSave += pow(RF_response[treeID][1][repMembrIndx[j]], 2.0);
    }
    for (i = 1; i <= actualCovariateCount; i++) {
      leftSize = 0;
      priorMembrIter = 0;
      splitLength = stackAndConstructSplitVector(treeID,
                                                 repMembrSize,
                                                 randomCovariateIndex[i], 
                                                 permissibleSplit[i], 
                                                 permissibleSplitSize[i],
                                                 & factorFlag,
                                                 & deterministicSplitFlag,
                                                 & mwcpSizeAbsolute,
                                                 & permissibleSplitPtr);
      if (factorFlag == FALSE) {
        for (j = 1; j <= repMembrSize; j++) {
          localSplitIndicator[j] = RIGHT;
        }
        sumRght      = sumRghtSave;
        sumRghtSqr   = sumRghtSqrSave; 
        sumLeft      = 0.0;
        sumLeftSqr   = 0.0;
        leftSizeIter = 0;
        rghtSizeIter = repMembrSize;
      }
      for (j = 1; j < splitLength; j++) {
        if (factorFlag == TRUE) {
          priorMembrIter = 0;
          leftSize = 0;
        }
        virtuallySplitNodeNew(treeID,
                              factorFlag,
                              mwcpSizeAbsolute,
                              randomCovariateIndex[i],
                              repMembrIndx,
                              repMembrIndxx[i],
                              repMembrSize,
                              permissibleSplitPtr,
                              j,
                              localSplitIndicator,
                              & leftSize,
                              priorMembrIter,
                              & currentMembrIter);
        rghtSize = repMembrSize - leftSize;
        if ((leftSize  >= (RF_minimumNodeSize)) && (rghtSize  >= (RF_minimumNodeSize))) {
          if (factorFlag == TRUE) {
            sumLeft = sumRght = 0.0;
            sumLeftSqr = sumRghtSqr = 0.0;
            for (k = 1; k <= repMembrSize; k++) {
              if (localSplitIndicator[k] == LEFT) {
                sumLeft    += RF_response[treeID][1][repMembrIndx[k]];
                sumLeftSqr += pow(RF_response[treeID][1][repMembrIndx[k]], 2.0);
              }
              else {
                sumRght    += RF_response[treeID][1][repMembrIndx[k]];
                sumRghtSqr += pow(RF_response[treeID][1][repMembrIndx[k]], 2.0);
              }
            }
          }
          else {
            for (k = leftSizeIter + 1; k < currentMembrIter; k++) {
              sumLeft    += RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]];
              sumLeftSqr += pow(RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]], 2.0);
              sumRght    -= RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]];
              sumRghtSqr -= pow(RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]], 2.0);
            }
            rghtSizeIter = rghtSizeIter - (currentMembrIter - (leftSizeIter + 1));
            leftSizeIter = currentMembrIter - 1; 
          }
          leftTemp = pow(sumLeft, 2.0) / pow(leftSize, 2.0);
          rghtTemp = pow(sumRght, 2.0) / pow(rghtSize, 2.0);
          leftTempSqr = sumLeftSqr / leftSize;
          rghtTempSqr = sumRghtSqr / rghtSize;
          delta = leftTemp + rghtTemp - leftTempSqr - rghtTempSqr;
          updateMaximumSplit(delta,
                             randomCovariateIndex[i],
                             j,
                             factorFlag,
                             mwcpSizeAbsolute,
                             & deltaMax,
                             splitParameterMax,
                             splitValueMaxCont,
                             splitValueMaxFactSize,
                             splitValueMaxFactPtr,
                             permissibleSplitPtr);
        }  
        if (factorFlag == FALSE) {
          if (rghtSize  < RF_minimumNodeSize) {
            j = splitLength;
          }
          else {
            priorMembrIter = currentMembrIter - 1;
          }
        }
      }  
      unstackSplitVector(treeID,
                         permissibleSplitSize[i],
                         splitLength,
                         factorFlag,
                         deterministicSplitFlag,
                         mwcpSizeAbsolute,
                         permissibleSplitPtr);
    }  
    unstackRandomCovariates(treeID,
                            repMembrSize, 
                            randomCovariateIndex, 
                            actualCovariateCount,
                            permissibleSplit, 
                            permissibleSplitSize,
                            repMembrIndxx);
    unstackSplitIndicator(repMembrSize, localSplitIndicator);
  }  
  result = summarizeSplitResult(*splitParameterMax, 
                                *splitValueMaxCont,
                                *splitValueMaxFactSize,
                                *splitValueMaxFactPtr,
                                 splitStatistic,
                                 deltaMax);
  return result;
}
char regressionHwghtSplit (uint    treeID, 
                           Node   *parent, 
                           uint   *repMembrIndx,
                           uint    repMembrSize,
                           uint   *allMembrIndx,
                           uint    allMembrSize,
                           uint   *splitParameterMax, 
                           double *splitValueMaxCont, 
                           uint   *splitValueMaxFactSize, 
                           uint  **splitValueMaxFactPtr,
                           double *splitStatistic) {
  uint    *randomCovariateIndex;
  double **permissibleSplit;
  uint    *permissibleSplitSize;
  uint   **repMembrIndxx;
  uint priorMembrIter, currentMembrIter;
  uint leftSizeIter, rghtSizeIter;
  uint leftSize, rghtSize;
  char *localSplitIndicator;
  double delta, deltaMax;
  double sumLeft, sumRght, sumRghtSave, sumRghtSqrSave, sumLeftSqr, sumRghtSqr;
  double leftTemp, rghtTemp, leftTempSqr, rghtTempSqr;
  uint splitLength;
  void *permissibleSplitPtr;
  char factorFlag;
  uint mwcpSizeAbsolute;
  char deterministicSplitFlag;
  char result;
  uint i, j, k;
  mwcpSizeAbsolute = 0;        
  sumLeft = sumRght      = 0;  
  sumLeftSqr             = 0;  
  sumRghtSqr             = 0;  
  leftSizeIter           = 0;  
  rghtSizeIter           = 0;  
  *splitParameterMax     = 0;
  *splitValueMaxFactSize = 0;
  *splitValueMaxFactPtr  = NULL;
  *splitValueMaxCont     = NA_REAL;
  deltaMax               = NA_REAL;
  if (repMembrSize >= (2 * RF_minimumNodeSize)) {
    result = TRUE;
  }
  else {
    result = FALSE;
  }
  if (result) {
    if (RF_maximumNodeDepth < 0) {
      result = TRUE;
    }
    else {
      if (parent -> depth < (uint) RF_maximumNodeDepth) {
        result = TRUE;
      }
      else {
        result = FALSE;
      }
    }
  }
  if (result) {
    result = getVariance(repMembrSize, repMembrIndx, RF_response[treeID][1], NULL, NULL);
  }
  if (result) {
    stackSplitIndicator(repMembrSize, & localSplitIndicator);
    uint actualCovariateCount = stackAndSelectRandomCovariates(treeID,
                                                               parent, 
                                                               repMembrIndx, 
                                                               repMembrSize, 
                                                               & randomCovariateIndex, 
                                                               & permissibleSplit, 
                                                               & permissibleSplitSize,
                                                               & repMembrIndxx);
    sumRghtSave = sumRghtSqrSave = 0.0;
    for (j = 1; j <= repMembrSize; j++) {
      sumRghtSave += RF_response[treeID][1][repMembrIndx[j]];
      sumRghtSqrSave += pow(RF_response[treeID][1][repMembrIndx[j]], 2.0);
    }
    for (i = 1; i <= actualCovariateCount; i++) {
      leftSize = 0;
      priorMembrIter = 0;
      splitLength = stackAndConstructSplitVector(treeID,
                                                 repMembrSize,
                                                 randomCovariateIndex[i], 
                                                 permissibleSplit[i], 
                                                 permissibleSplitSize[i],
                                                 & factorFlag,
                                                 & deterministicSplitFlag,
                                                 & mwcpSizeAbsolute,
                                                 & permissibleSplitPtr);
      if (factorFlag == FALSE) {
        for (j = 1; j <= repMembrSize; j++) {
          localSplitIndicator[j] = RIGHT;
        }
        sumRght      = sumRghtSave;
        sumRghtSqr   = sumRghtSqrSave; 
        sumLeft      = 0.0;
        sumLeftSqr   = 0.0;
        leftSizeIter = 0;
        rghtSizeIter = repMembrSize;
      }
      for (j = 1; j < splitLength; j++) {
        if (factorFlag == TRUE) {
          priorMembrIter = 0;
          leftSize = 0;
        }
        virtuallySplitNodeNew(treeID,
                              factorFlag,
                              mwcpSizeAbsolute,
                              randomCovariateIndex[i],
                              repMembrIndx,
                              repMembrIndxx[i],
                              repMembrSize,
                              permissibleSplitPtr,
                              j,
                              localSplitIndicator,
                              & leftSize,
                              priorMembrIter,
                              & currentMembrIter);
        rghtSize = repMembrSize - leftSize;
        if ((leftSize  >= (RF_minimumNodeSize)) && (rghtSize  >= (RF_minimumNodeSize))) {
          if (factorFlag == TRUE) {
            sumLeft = sumRght = 0.0;
            sumLeftSqr = sumRghtSqr = 0.0;
            for (k = 1; k <= repMembrSize; k++) {
              if (localSplitIndicator[k] == LEFT) {
                sumLeft    += RF_response[treeID][1][repMembrIndx[k]];
                sumLeftSqr += pow(RF_response[treeID][1][repMembrIndx[k]], 2.0);
              }
              else {
                sumRght    += RF_response[treeID][1][repMembrIndx[k]];
                sumRghtSqr += pow(RF_response[treeID][1][repMembrIndx[k]], 2.0);
              }
            }
          }
          else {
            for (k = leftSizeIter + 1; k < currentMembrIter; k++) {
              sumLeft    += RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]];
              sumLeftSqr += pow(RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]], 2.0);
              sumRght    -= RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]];
              sumRghtSqr -= pow(RF_response[treeID][1][repMembrIndx[repMembrIndxx[i][k]]], 2.0);
            }
            rghtSizeIter = rghtSizeIter - (currentMembrIter - (leftSizeIter + 1));
            leftSizeIter = currentMembrIter - 1; 
          }
          leftTemp = pow(sumLeft, 2.0) / pow (repMembrSize, 2.0);
          rghtTemp = pow(sumRght, 2.0) / pow (repMembrSize, 2.0);
          leftTempSqr = sumLeftSqr * leftSize / pow (repMembrSize, 2.0);
          rghtTempSqr = sumRghtSqr * rghtSize / pow (repMembrSize, 2.0);
          delta = leftTemp + rghtTemp - leftTempSqr - rghtTempSqr;
          updateMaximumSplit(delta,
                             randomCovariateIndex[i],
                             j,
                             factorFlag,
                             mwcpSizeAbsolute,
                             & deltaMax,
                             splitParameterMax,
                             splitValueMaxCont,
                             splitValueMaxFactSize,
                             splitValueMaxFactPtr,
                             permissibleSplitPtr);
        }  
        if (factorFlag == FALSE) {
          if (rghtSize  < RF_minimumNodeSize) {
            j = splitLength;
          }
          else {
            priorMembrIter = currentMembrIter - 1;
          }
        }
      }  
      unstackSplitVector(treeID,
                         permissibleSplitSize[i],
                         splitLength,
                         factorFlag,
                         deterministicSplitFlag,
                         mwcpSizeAbsolute,
                         permissibleSplitPtr);
    }  
    unstackRandomCovariates(treeID,
                            repMembrSize, 
                            randomCovariateIndex, 
                            actualCovariateCount,
                            permissibleSplit, 
                            permissibleSplitSize,
                            repMembrIndxx);
    unstackSplitIndicator(repMembrSize, localSplitIndicator);
  }  
  result = summarizeSplitResult(*splitParameterMax, 
                                *splitValueMaxCont,
                                *splitValueMaxFactSize,
                                *splitValueMaxFactPtr,
                                 splitStatistic,
                                 deltaMax);
  return result;
}
char mvRegressionSplit (uint    treeID, 
                        Node   *parent, 
                        uint   *repMembrIndx,
                        uint    repMembrSize,
                        uint   *allMembrIndx,
                        uint    allMembrSize,
                        uint   *splitParameterMax, 
                        double *splitValueMaxCont, 
                        uint   *splitValueMaxFactSize, 
                        uint  **splitValueMaxFactPtr,
                        double *splitStatistic) {
  uint    *randomCovariateIndex;
  double **permissibleSplit;
  uint    *permissibleSplitSize;
  uint   **repMembrIndxx;
  uint priorMembrIter, currentMembrIter;
  uint leftSizeIter, rghtSizeIter;
  uint leftSize, rghtSize;
  char   *purity;
  double *mean;
  double *variance;
  char    puritySummary;
  char *localSplitIndicator;
  double delta, deltaMax;
  double *sumLeft, *sumRght, *sumRghtSave, sumLeftSqr, sumRghtSqr;
  uint splitLength;
  void *permissibleSplitPtr;
  char factorFlag;
  uint mwcpSizeAbsolute;
  char deterministicSplitFlag;
  char result;
  uint i, j, k, r;
  mwcpSizeAbsolute       = 0;  
  sumLeft = sumRght      = 0;  
  leftSizeIter           = 0;  
  rghtSizeIter           = 0;  
  *splitParameterMax     = 0;
  *splitValueMaxFactSize = 0;
  *splitValueMaxFactPtr  = NULL;
  *splitValueMaxCont     = NA_REAL;
  deltaMax               = NA_REAL;
  if (repMembrSize >= (2 * RF_minimumNodeSize)) {
    result = TRUE;
  }
  else {
    result = FALSE;
  }
  if (result) {
    if (RF_maximumNodeDepth < 0) {
      result = TRUE;
    }
    else {
      if (parent -> depth < (uint) RF_maximumNodeDepth) {
        result = TRUE;
      }
      else {
        result = FALSE;
      }
    }
  }
  purity   = cvector(1, RF_rSize); 
  mean     = dvector(1, RF_rSize);
  variance = dvector(1, RF_rSize);
  sumLeft      = dvector(1, RF_rSize);
  sumRght      = dvector(1, RF_rSize);
  sumRghtSave  = dvector(1, RF_rSize);
  if (result) {
    puritySummary = FALSE;
    for (r = 1; r <= RF_rSize; r++) {
      purity[r] = getVariance(repMembrSize, repMembrIndx, RF_response[treeID][r], &mean[r], &variance[r]);
      puritySummary = puritySummary | purity[r];
    }
    result = puritySummary;
  }
  if (result) {
    stackSplitIndicator(repMembrSize, & localSplitIndicator);
    uint actualCovariateCount = stackAndSelectRandomCovariates(treeID,
                                                               parent, 
                                                               repMembrIndx, 
                                                               repMembrSize, 
                                                               & randomCovariateIndex, 
                                                               & permissibleSplit, 
                                                               & permissibleSplitSize,
                                                               & repMembrIndxx);
    for (r = 1; r <= RF_rSize; r++) {
      sumRghtSave[r] = 0.0;
      for (j = 1; j <= repMembrSize; j++) {
        sumRghtSave[r] += RF_response[treeID][r][repMembrIndx[j]] - mean[r];
      }
    }
    for (i = 1; i <= actualCovariateCount; i++) {
      leftSize = 0;
      priorMembrIter = 0;
      splitLength = stackAndConstructSplitVector(treeID,
                                                 repMembrSize,
                                                 randomCovariateIndex[i], 
                                                 permissibleSplit[i], 
                                                 permissibleSplitSize[i],
                                                 & factorFlag,
                                                 & deterministicSplitFlag,
                                                 & mwcpSizeAbsolute,
                                                 & permissibleSplitPtr);
      if (factorFlag == FALSE) {
        for (j = 1; j <= repMembrSize; j++) {
          localSplitIndicator[j] = RIGHT;
        }
        for (r = 1; r <= RF_rSize; r++) {
          sumRght[r]      = sumRghtSave[r];
          sumLeft[r]      = 0.0;
        }
        leftSizeIter = 0;
        rghtSizeIter = repMembrSize;
      }
      for (j = 1; j < splitLength; j++) {
        if (factorFlag == TRUE) {
          priorMembrIter = 0;
          leftSize = 0;
        }
        virtuallySplitNodeNew(treeID,
                              factorFlag,
                              mwcpSizeAbsolute,
                              randomCovariateIndex[i],
                              repMembrIndx,
                              repMembrIndxx[i],
                              repMembrSize,
                              permissibleSplitPtr,
                              j,
                              localSplitIndicator,
                              & leftSize,
                              priorMembrIter,
                              & currentMembrIter);
        rghtSize = repMembrSize - leftSize;
        if ((leftSize  >= (RF_minimumNodeSize)) && (rghtSize  >= (RF_minimumNodeSize))) {
          delta = 0.0;
          for (r = 1; r <= RF_rSize; r++) {
            if (purity[r]) {
              if (factorFlag == TRUE) {
                sumLeft[r] = sumRght[r] = 0.0;
                for (k = 1; k <= repMembrSize; k++) {
                  if (localSplitIndicator[k] == LEFT) {
                    sumLeft[r] += RF_response[treeID][r][repMembrIndx[k]] - mean[r];
                  }
                  else {
                    sumRght[r] += RF_response[treeID][r][repMembrIndx[k]] - mean[r];
                  }
                }
              }
              else {
                for (k = leftSizeIter + 1; k < currentMembrIter; k++) {
                  sumLeft[r] += RF_response[treeID][r][repMembrIndx[repMembrIndxx[i][k]]] - mean[r];
                  sumRght[r] -= RF_response[treeID][r][repMembrIndx[repMembrIndxx[i][k]]] - mean[r];
                }
              }
              sumLeftSqr = pow (sumLeft[r], 2.0) / (leftSize * variance[r]);
              sumRghtSqr = pow (sumRght[r], 2.0) / (rghtSize * variance[r]);
              delta += sumLeftSqr + sumRghtSqr;
            } 
          }  
          if (factorFlag == FALSE) {
            rghtSizeIter = rghtSizeIter - (currentMembrIter - (leftSizeIter + 1));
            leftSizeIter = currentMembrIter - 1; 
          }
          updateMaximumSplit(delta,
                             randomCovariateIndex[i],
                             j,
                             factorFlag,
                             mwcpSizeAbsolute,
                             & deltaMax,
                             splitParameterMax,
                             splitValueMaxCont,
                             splitValueMaxFactSize,
                             splitValueMaxFactPtr,
                             permissibleSplitPtr);
        }  
        if (factorFlag == FALSE) {
          if (rghtSize  < RF_minimumNodeSize) {
            j = splitLength;
          }
          else {
            priorMembrIter = currentMembrIter - 1;
          }
        }
      }  
      unstackSplitVector(treeID,
                         permissibleSplitSize[i],
                         splitLength,
                         factorFlag,
                         deterministicSplitFlag,
                         mwcpSizeAbsolute,
                         permissibleSplitPtr);
    }  
    unstackRandomCovariates(treeID,
                            repMembrSize, 
                            randomCovariateIndex, 
                            actualCovariateCount,
                            permissibleSplit, 
                            permissibleSplitSize,
                            repMembrIndxx);
    unstackSplitIndicator(repMembrSize, localSplitIndicator);
  }  
  free_cvector(purity,   1, RF_rSize); 
  free_dvector(mean,     1, RF_rSize);
  free_dvector(variance, 1, RF_rSize);
  free_dvector(sumLeft, 1, RF_rSize);
  free_dvector(sumRght, 1, RF_rSize);
  free_dvector(sumRghtSave, 1, RF_rSize);
  result = summarizeSplitResult(*splitParameterMax, 
                                *splitValueMaxCont,
                                *splitValueMaxFactSize,
                                *splitValueMaxFactPtr,
                                 splitStatistic,
                                 deltaMax);
  return result;
}
