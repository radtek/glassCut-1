/*============================================================================*/
/*                        IMAQ Vision Machine Vision                          */
/*----------------------------------------------------------------------------*/
/*    Copyright (c) National Instruments 2001.  All Rights Reserved.          */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Title:       NIMachineVision.c                                             */
/* Purpose:     Provides open source functional extensions for IMAQ Vision.   */
/* Note:        If you need to make changes to this file, create a separate   */
/*              copy in another directory and edit that file. Otherwise, you  */
/*              may lose your changes when upgrading to newer versions of     */
/*              IMAQ Vision.                                                  */
/*                                                                            */
/*============================================================================*/
#include "nimachinevision.h"

#pragma warning(disable: 4090)
#pragma warning(disable: 4996)
#pragma warning(disable: 4018)
//////////////////////////////////////////////////////////////////////////////
//  Coordinate Transform
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  imaqFindTransformPattern
//
//  Description:
//      Updates or initializes a transform based on the position of a pattern.
//
//  Parameters:
//      image       -   The image which the function uses to compute the 
//                      coordinate transform.
//      pattern     -   The pattern image which the function attempts to locate.
//      transform   -   The coordinate transform the function updates based on 
//                      the location and position of the pattern.
//      searchRect  -   The coordinate location of the rectangular search area 
//                      the function looks in for the pattern.
//      mode        -   Specifies how the function updates the coordinate 
//                      transform.
//      options     -   Defines the parameters of the algorithm the function 
//                      uses to locate the pattern and the information the 
//                      function overlays to the image.
//      report      -   Upon return, a report describing the location of the 
//                      pattern corresponding to the main axis and the seconday 
//                      axis.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqFindTransformPattern(Image* image, Image* pattern, CoordinateTransform2* transform, RotatedRect searchRect, FindTransformMode mode, const FindTransformPatternOptions* options, AxisReport* report) {

    int matchingPresent;
    MatchPatternOptions matchOptions;
    PatternMatch* match;
    int numMatches;

    // Verify that the transform is not NULL
    if( transform == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqFindTransformPatternErrorString, NULL );
        return FALSE;
    }

    // Check that the pattern image has the pattern matching information we need
    if ( imaqIsVisionInfoPresent( pattern, IMAQ_PATTERN_MATCHING_INFO, &matchingPresent ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformPatternErrorString, NULL );
        return FALSE;
    }

    // If there is no matching information, learn the pattern
    if ( !matchingPresent ) {
        LearningMode mode = IMAQ_LEARN_SHIFT_INFORMATION;
        if ( options && options->matchMode == IMAQ_MATCH_ROTATION_INVARIANT )
            mode = IMAQ_LEARN_ROTATION_INFORMATION;
        if ( imaqLearnPattern2( pattern, mode, NULL ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindTransformPatternErrorString, NULL );
            return FALSE;
        }
    }

    // Set the match pattern options
    if ( options ) {
        matchOptions.mode = options->matchMode;
        matchOptions.minMatchScore = (float)options->minMatchScore;
        matchOptions.subpixelAccuracy = options->subpixelAccuracy;
        matchOptions.angleRanges = options->angleRanges;
        matchOptions.numRanges = options->numRanges;
    }
    else {
        matchOptions.mode = DEFAULT_MATCH_MODE;
        matchOptions.minMatchScore = FIND_TRAN_MIN_MATCH_SCORE;
        matchOptions.subpixelAccuracy = DEFAULT_SUBPIXEL;
        matchOptions.angleRanges = DEFAULT_ANGLE_RANGES;
        matchOptions.numRanges = DEFAULT_NUM_RANGES;
    }
    matchOptions.minContrast = DEFAULT_MIN_CONTRAST;
    matchOptions.numMatchesRequested = DEFAULT_NUM_MATCHES;
    matchOptions.matchFactor = DEFAULT_MATCH_FACTOR;

    // Look for the pattern
    if (( match = imaqMatchPattern2( image, pattern, &matchOptions, NULL, imaqMakeRectFromRotatedRect(searchRect), &numMatches )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformPatternErrorString, NULL );
        return FALSE;
    }

    // If there was no match, return an error   
    if ( numMatches == 0 ) {
        imaqPrepareForExit( ERR_COORDSYS_NOT_FOUND, imaqFindTransformPatternErrorString, match, NULL );
        return FALSE;
    }

    // Update the transform based on the mode
    transform->measurementSystem.origin = match->corner[3]; 
    transform->measurementSystem.angle = match->rotation;   
    transform->measurementSystem.axisOrientation = IMAQ_DIRECT; 
    if ( mode == IMAQ_FIND_REFERENCE ) {
        transform->referenceSystem.origin = transform->measurementSystem.origin;    
        transform->referenceSystem.angle = transform->measurementSystem.angle;  
        transform->referenceSystem.axisOrientation = transform->measurementSystem.axisOrientation;  
    }

    // Set up the axis report (if requested)
    if ( report ) {
        report->origin = match->corner[3];
        report->mainAxisEnd = match->corner[2];
        report->secondaryAxisEnd = match->corner[0];
    }

    // Overlay the results
    if ( imaqOverlayFindTransformPattern ( image, searchRect, match, options ) ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformPatternErrorString, match, NULL );
        return FALSE;
    }

    // Dispose of the pattern match
    if ( imaqDispose(match) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformPatternErrorString, NULL );
        return FALSE;
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqFindTransformRect
//
//  Description:
//      Updates or initializes a transform based on the egdes of an object.
//
//  Parameters:
//      image       -   The image which the function uses to compute the 
//                      coordinate transform.
//      searchRect  -   The coordinate location of the rectangular search area 
//                      the function looks in for the edges.
//      transform   -   The coordinate transform the function updates based on 
//                      the location and rotation of the object.
//      mode        -   Specifies how the function updates the coordinate 
//                      transform.
//      options     -   Defines the parameters of the algorithm the function 
//                      uses to locate the pattern and the information the 
//                      function overlays to the image.
//      report      -   Upon return, a report describing the location of the 
//                      pattern corresponding to the main axis and the seconday 
//                      axis.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqFindTransformRect(Image* image, RotatedRect searchRect, CoordinateTransform2* transform, FindTransformMode mode, const FindTransformRectOptions* options, AxisReport* report) {

    StraightEdgeReport* reportToReturn = NULL;
    ROI* mainROI;
    ROI* secondaryROI;
    ContourID rectID;
    RakeOptions optionsToUse;
    RakeReport* mainReportToProcess;
    RakeReport* secondaryReportToProcess;
    RakeDirection mainAxisDirection, secondaryAxisDirection;
    BestLine* fitLine = NULL;
    FitLineOptions fitLineOptions;
    double angle;
    CoordinateTransform2 secondaryTransform;
    PointFloat origin = {0, 0};
    PointFloat mainAxisEnd = {0, 0};
    PointFloat secondaryAxisEnd = {0, 0};
    PointFloat boxEdge1Start, boxEdge1End, boxEdge2Start, boxEdge2End, transformOrigin, tempEdge, closetFirstEdge;
    Rect mainROIBoundingBox, secondarySearchRect;

    // Verify that the transform is not NULL
    if( transform == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqFindTransformRectErrorString, NULL );
        return FALSE;
    }

    // Verify that the two rake directions are perpendicular
    if ( options ) {
        switch (options->mainAxisDirection) {
            case IMAQ_LEFT_TO_RIGHT:
            case IMAQ_RIGHT_TO_LEFT:
                if(options->secondaryAxisDirection != IMAQ_TOP_TO_BOTTOM && options->secondaryAxisDirection != IMAQ_BOTTOM_TO_TOP ) {
                    imaqPrepareForExit( ERR_INVALID_RAKE_DIRECTION, imaqFindTransformRectErrorString, NULL );
                    return FALSE;
                }
                break;
            case IMAQ_TOP_TO_BOTTOM:
            case IMAQ_BOTTOM_TO_TOP:
                if(options->secondaryAxisDirection != IMAQ_LEFT_TO_RIGHT && options->secondaryAxisDirection != IMAQ_RIGHT_TO_LEFT ) {
                    imaqPrepareForExit( ERR_INVALID_RAKE_DIRECTION, imaqFindTransformRectErrorString, NULL );
                    return FALSE;
                }
                break;
            default:
                imaqPrepareForExit( ERR_INVALID_RAKE_DIRECTION, imaqFindTransformRectErrorString, NULL );
                return FALSE;
        }
    }

    // Create an ROI, place the rotated rect contour inside it and set the contour color
    mainROI = imaqCreateROI();
    if ( mainROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, NULL );
        return FALSE;
    }
    if ( (rectID = imaqAddRotatedRectContour2 (mainROI, searchRect)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, NULL );
        return FALSE;
    }
    if ( imaqSetContourColor( mainROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, NULL );
        return FALSE;
    }

    // Use the supplied rake options (if available) or use default options
    if( options ) {
        optionsToUse.threshold = options->threshold;
        optionsToUse.width = options->width;
        optionsToUse.steepness = options->steepness;
        optionsToUse.subsamplingRatio = options->subsamplingRatio;
        mainAxisDirection = options->mainAxisDirection;
        secondaryAxisDirection = options->secondaryAxisDirection;
        
    }
    else {
        optionsToUse.threshold = DEFAULT_THRESHOLD;
        optionsToUse.width = DEFAULT_WIDTH;
        optionsToUse.steepness = DEFAULT_STEEPNESS;
        optionsToUse.subsamplingRatio = DEFAULT_SUBSAMPLING_RATIO;
        mainAxisDirection = DEFAULT_MAIN_AXIS_DIR;
        secondaryAxisDirection = DEFAULT_SEC_AXIS_DIR;
    }

    // Set the remaining options
    optionsToUse.subpixelType = IMAQ_CUBIC_SPLINE;
    optionsToUse.subpixelDivisions = DEFAULT_SUBPIXEL_DIVISIONS;

    // Calculate the edge locations
    if ( (mainReportToProcess = imaqRake( image, mainROI, mainAxisDirection, IMAQ_FIRST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, NULL );
        return FALSE;
    }

    // Verify there we at least two first edges
    if ( mainReportToProcess->numFirstEdges < 2 ) {
        imaqPrepareForExit( ERR_COORDSYS_NOT_FOUND, imaqFindTransformRectErrorString, mainROI, mainReportToProcess, NULL );
        return FALSE;
    }
    
    // Fit a line to the detected edges
    fitLineOptions.minScore       = DEFAULT_MIN_LINE_SCORE;
    fitLineOptions.pixelRadius    = DEFAULT_PIXEL_RADIUS;
    fitLineOptions.numRefinements = DEFAULT_REFINEMENTS;
    if ( (fitLine = imaqFitLine( mainReportToProcess->firstEdges, mainReportToProcess->numFirstEdges, &fitLineOptions )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, NULL );
        return FALSE;
    }
    
    // Verify the validity of the fit line
    if( !fitLine->valid ) {
        imaqPrepareForExit( ERR_COORDSYS_NOT_FOUND, imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }

    // Calculate the angle of the coordinate system 
    angle = atan2( -fitLine->equation.a, fitLine->equation.b ) * 180 / IMAQ_PI;
    if ( mainAxisDirection == IMAQ_LEFT_TO_RIGHT || mainAxisDirection == IMAQ_RIGHT_TO_LEFT ) {
        if ((fitLine->equation.a <= 0 && fitLine->equation.b >= 0) || (fitLine->equation.a >= 0 && fitLine->equation.b <= 0))
            angle = 90 - angle;   
        else
            angle = 270 - angle;
    }
    else 
        angle = -angle;

    // Get the bounding information for the search rect
    if ( imaqGetROIBoundingBox (mainROI, &mainROIBoundingBox) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }

    if ( mainAxisDirection == IMAQ_LEFT_TO_RIGHT || mainAxisDirection == IMAQ_RIGHT_TO_LEFT ) {
        boxEdge1Start.x = (float)mainROIBoundingBox.left;
        boxEdge1Start.y = (float)mainROIBoundingBox.top;
        boxEdge1End.x = (float)(mainROIBoundingBox.left + mainROIBoundingBox.width);
        boxEdge1End.y = (float)mainROIBoundingBox.top;
        boxEdge2Start.x = (float)mainROIBoundingBox.left;
        boxEdge2Start.y = (float)(mainROIBoundingBox.top + mainROIBoundingBox.height);
        boxEdge2End.x = (float)(mainROIBoundingBox.left + mainROIBoundingBox.width);
        boxEdge2End.y = (float)(mainROIBoundingBox.top + mainROIBoundingBox.height);
    }
    else {
        boxEdge1Start.x = (float)mainROIBoundingBox.left;
        boxEdge1Start.y = (float)mainROIBoundingBox.top;
        boxEdge1End.x = (float)mainROIBoundingBox.left;
        boxEdge1End.y = (float)(mainROIBoundingBox.top  + mainROIBoundingBox.height);
        boxEdge2Start.x = (float)(mainROIBoundingBox.left  + mainROIBoundingBox.width);
        boxEdge2Start.y = (float)mainROIBoundingBox.top;
        boxEdge2End.x = (float)(mainROIBoundingBox.left + mainROIBoundingBox.width);
        boxEdge2End.y = (float)(mainROIBoundingBox.top + mainROIBoundingBox.height);
    }

    // Calculate the main axis
    if ( imaqGetIntersection( boxEdge1Start, boxEdge1End, fitLine->start, fitLine->end, &origin ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }
    if ( imaqGetIntersection( boxEdge2Start, boxEdge2End, fitLine->start, fitLine->end, &mainAxisEnd ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }

    // Calculate the transform origin
    transformOrigin.x = (origin.x + mainAxisEnd.x) / 2;
    transformOrigin.y = (origin.y + mainAxisEnd.y) / 2;
    
    // Adjust the search rectangle for the secondary axis
    if ( mainAxisDirection == IMAQ_LEFT_TO_RIGHT || mainAxisDirection == IMAQ_TOP_TO_BOTTOM ) {
        if ( imaqGetPerpendicularLine( boxEdge1Start, boxEdge1End, transformOrigin, NULL, &tempEdge, NULL) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, NULL );
            return FALSE;
        }
        secondarySearchRect.left = (int)tempEdge.x;
        secondarySearchRect.top = (int)tempEdge.y;
        secondarySearchRect.width = (int)(boxEdge2End.x - tempEdge.x);
        secondarySearchRect.height = (int)(boxEdge2End.y - tempEdge.y);
    }
    else {
        if ( imaqGetPerpendicularLine( boxEdge2Start, boxEdge2End, transformOrigin, NULL, &tempEdge, NULL) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, NULL );
            return FALSE;
        }
        secondarySearchRect.left = (int)boxEdge1Start.x;
        secondarySearchRect.top = (int)boxEdge1Start.y;
        secondarySearchRect.width = (int)(tempEdge.x - boxEdge1Start.x);
        secondarySearchRect.height = (int)(tempEdge.y - boxEdge1Start.y);
    }
    
    secondaryROI = imaqCreateROI();
    if ( secondaryROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }
    if ( imaqAddRectContour (secondaryROI, secondarySearchRect) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, NULL );
        return FALSE;
    }

    // Transform the search rectangle for the secondary axis search
    secondaryTransform.referenceSystem.origin = transformOrigin;
    secondaryTransform.measurementSystem.origin = transformOrigin;
    secondaryTransform.referenceSystem.angle = 0;
    secondaryTransform.measurementSystem.angle = (float)angle;
    secondaryTransform.referenceSystem.axisOrientation = IMAQ_DIRECT;
    secondaryTransform.measurementSystem.axisOrientation = IMAQ_DIRECT;

    if ( imaqTransformROI2( secondaryROI, &secondaryTransform.referenceSystem, &secondaryTransform.measurementSystem ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, NULL );
        return FALSE;
    }
    
    // Locate the edges on the secondary axis.
    if ( (secondaryReportToProcess = imaqRake( image, secondaryROI, secondaryAxisDirection, IMAQ_FIRST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, NULL );
        return FALSE;
    }

    // Verify there is at least one edge
    if ( secondaryReportToProcess->numFirstEdges < 1 ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }

    // Find the closest first edge
    if ( imaqFindExtremeEdge( secondaryReportToProcess, TRUE, &closetFirstEdge ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }
    
    // Set the position of the secondary axis, adjusting the origin
    if ( imaqGetPerpendicularLine( fitLine->start, fitLine->end, closetFirstEdge, &secondaryAxisEnd, &origin, NULL) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }

    // Update the transform based on the mode
    transform->measurementSystem.origin = origin;   
    transform->measurementSystem.angle = (float)angle; 
    if ( imaqSetAxisOrienation( mainAxisDirection, secondaryAxisDirection, &transform->measurementSystem.axisOrientation ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }
    if ( mode == IMAQ_FIND_REFERENCE ) {
        transform->referenceSystem.origin = transform->measurementSystem.origin;    
        transform->referenceSystem.angle = transform->measurementSystem.angle;  
        transform->referenceSystem.axisOrientation = transform->measurementSystem.axisOrientation;  
    }
    
    // Adjust the axis so it fits in the provided bounding box
    if ( imaqFitAxisReportToRect( mainROIBoundingBox, mainAxisDirection, secondaryAxisDirection, origin, &mainAxisEnd, &secondaryAxisEnd ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }

    // Set up the axis report (if requested)
    if ( report ) {
        report->origin = origin;
        report->mainAxisEnd = mainAxisEnd;
        report->secondaryAxisEnd = secondaryAxisEnd;
    }

    // Overlay the results on the image
    if ( imaqOverlayFindTransformRects( image, mainROI, mainReportToProcess, secondaryReportToProcess, origin, mainAxisEnd, secondaryAxisEnd, options, NULL ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }
    
    // Dispose of the allocated structures
    if ( imaqDispose(secondaryReportToProcess) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, NULL );
        return FALSE;
    }
    if ( imaqDispose(secondaryROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }
    if ( imaqDispose(fitLine) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, mainReportToProcess, NULL );
        return FALSE;
    }
    if ( imaqDispose(mainReportToProcess) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, mainROI, NULL );
        return FALSE;
    }
    if ( imaqDispose(mainROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectErrorString, NULL );
        return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqFindTransformRects
//
//  Description:
//      Updates or initializes a transform based on the egdes of an object.
//
//  Parameters:
//      image           -   The image which the function uses to compute the 
//                          coordinate transform.
//      primaryRect     -   The coordinate location of the first rectangular 
//                          search area the function looks in for the edges.
//      secondaryRect   -   The coordinate location of the second rectangular 
//                          search area the function looks in for the edges.
//      transform       -   The coordinate transform the function updates based 
//                          on the location and rotation of the object.
//      mode            -   Specifies how the function updates the coordinate 
//                          transform.
//      options         -   Defines the parameters of the algorithm the function 
//                          uses to locate the pattern and the information the 
//                          function overlays to the image.
//      report          -   Upon return, a report describing the location of the 
//                          edges corresponding to the main axis and the 
//                          seconday axis.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqFindTransformRects(Image* image, RotatedRect primaryRect, RotatedRect secondaryRect, CoordinateTransform2* transform, FindTransformMode mode, const FindTransformRectsOptions* options, AxisReport* report) {

    StraightEdgeReport* reportToReturn = NULL;
    ROI* mainROI;
    ROI* secondaryROI;
    ContourID rectID;
    RakeOptions optionsToUse;
    RakeReport* mainReportToProcess;
    RakeReport* secondaryReportToProcess;
    RakeDirection mainAxisDirection, secondaryAxisDirection;
    BestLine* fitLine = NULL;
    FitLineOptions fitLineOptions;
    double angle;
    PointFloat origin = {0, 0};
    PointFloat mainAxisEnd = {0, 0};
    PointFloat secondaryAxisEnd = {0, 0};
    PointFloat boxEdge1Start, boxEdge1End, boxEdge2Start, boxEdge2End, closetFirstEdge;
    Rect mainROIBoundingBox;

    // Verify that the transform is not NULL
    if( transform == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqFindTransformRectsErrorString, NULL );
        return FALSE;
    }

    // Verify that the two rake directions are perpendicular
    if ( options ) {
        switch (options->mainAxisDirection) {
            case IMAQ_LEFT_TO_RIGHT:
            case IMAQ_RIGHT_TO_LEFT:
                if(options->secondaryAxisDirection != IMAQ_TOP_TO_BOTTOM && options->secondaryAxisDirection != IMAQ_BOTTOM_TO_TOP ) {
                    imaqPrepareForExit( ERR_INVALID_RAKE_DIRECTION, imaqFindTransformRectErrorString, NULL );
                    return FALSE;
                }
                break;
            case IMAQ_TOP_TO_BOTTOM:
            case IMAQ_BOTTOM_TO_TOP:
                if(options->secondaryAxisDirection != IMAQ_LEFT_TO_RIGHT && options->secondaryAxisDirection != IMAQ_RIGHT_TO_LEFT ) {
                    imaqPrepareForExit( ERR_INVALID_RAKE_DIRECTION, imaqFindTransformRectsErrorString, NULL );
                    return FALSE;
                }
                break;
            default:
                imaqPrepareForExit( ERR_INVALID_RAKE_DIRECTION, imaqFindTransformRectsErrorString, NULL );
                return FALSE;
        }
    }

    // Create an ROI, place the rotated rect contour inside it and set the contour color
    mainROI = imaqCreateROI();
    if ( mainROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, NULL );
        return FALSE;
    }
    if ( (rectID = imaqAddRotatedRectContour2 (mainROI, primaryRect)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, NULL );
        return FALSE;
    }
    if ( imaqSetContourColor( mainROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, NULL );
        return FALSE;
    }

    // Use the supplied rake options (if available) or use default options
    if( options ) {          
        optionsToUse.threshold = options->primaryThreshold;
        optionsToUse.width = options->primaryWidth;
        optionsToUse.steepness = options->primarySteepness;
        optionsToUse.subsamplingRatio = options->primarySubsamplingRatio;
        mainAxisDirection = options->mainAxisDirection;
        secondaryAxisDirection = options->secondaryAxisDirection;
        
    }
    else {
        optionsToUse.threshold = DEFAULT_THRESHOLD;
        optionsToUse.width = DEFAULT_WIDTH;
        optionsToUse.steepness = DEFAULT_STEEPNESS;
        optionsToUse.subsamplingRatio = DEFAULT_SUBSAMPLING_RATIO;
        mainAxisDirection = DEFAULT_MAIN_AXIS_DIR;
        secondaryAxisDirection = DEFAULT_SEC_AXIS_DIR;
    }

    // Set the remaining options
    optionsToUse.subpixelType = IMAQ_CUBIC_SPLINE;
    optionsToUse.subpixelDivisions = DEFAULT_SUBPIXEL_DIVISIONS;

    // Calculate the edge locations
    if ( (mainReportToProcess = imaqRake( image, mainROI, mainAxisDirection, IMAQ_FIRST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, NULL );
        return FALSE;
    }

    // Verify there we at least two first edges
    if ( mainReportToProcess->numFirstEdges < 2 ) {
        imaqPrepareForExit( ERR_COORDSYS_NOT_FOUND, imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, NULL );
        return FALSE;
    }
    
    // Fit a line to the detected edges
    fitLineOptions.minScore       = DEFAULT_MIN_LINE_SCORE;
    fitLineOptions.pixelRadius    = DEFAULT_PIXEL_RADIUS;
    fitLineOptions.numRefinements = DEFAULT_REFINEMENTS;
    if ( (fitLine = imaqFitLine( mainReportToProcess->firstEdges, mainReportToProcess->numFirstEdges, &fitLineOptions )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, NULL );
        return FALSE;
    }
    
    // Verify the validity of the fit line
    if( !fitLine->valid ) {
        imaqPrepareForExit( ERR_COORDSYS_NOT_FOUND, imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }

    // Calculate the angle of the coordinate system 
    angle = atan2( -fitLine->equation.a, fitLine->equation.b ) * 180 / IMAQ_PI;
    if ( mainAxisDirection == IMAQ_LEFT_TO_RIGHT || mainAxisDirection == IMAQ_RIGHT_TO_LEFT ) {
        if ((fitLine->equation.a <= 0 && fitLine->equation.b >= 0) || (fitLine->equation.a >= 0 && fitLine->equation.b <= 0))
            angle = 90 - angle;
        else
            angle = 270 - angle;
    }
    else 
        angle = -angle;

    // Get the bounding information for the search rect
    if ( imaqGetROIBoundingBox (mainROI, &mainROIBoundingBox) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }

    if ( mainAxisDirection == IMAQ_LEFT_TO_RIGHT || mainAxisDirection == IMAQ_RIGHT_TO_LEFT ) {
        boxEdge1Start.x = (float)mainROIBoundingBox.left;
        boxEdge1Start.y = (float)mainROIBoundingBox.top;
        boxEdge1End.x = (float)(mainROIBoundingBox.left + mainROIBoundingBox.width);
        boxEdge1End.y = (float)mainROIBoundingBox.top;
        boxEdge2Start.x = (float)mainROIBoundingBox.left;
        boxEdge2Start.y = (float)(mainROIBoundingBox.top + mainROIBoundingBox.height);
        boxEdge2End.x = (float)(mainROIBoundingBox.left + mainROIBoundingBox.width);
        boxEdge2End.y = (float)(mainROIBoundingBox.top + mainROIBoundingBox.height);
    }
    else {
        boxEdge1Start.x = (float)mainROIBoundingBox.left;
        boxEdge1Start.y = (float)mainROIBoundingBox.top;
        boxEdge1End.x = (float)mainROIBoundingBox.left;
        boxEdge1End.y = (float)(mainROIBoundingBox.top  + mainROIBoundingBox.height);
        boxEdge2Start.x = (float)(mainROIBoundingBox.left  + mainROIBoundingBox.width);
        boxEdge2Start.y = (float)mainROIBoundingBox.top;
        boxEdge2End.x = (float)(mainROIBoundingBox.left + mainROIBoundingBox.width);
        boxEdge2End.y = (float)(mainROIBoundingBox.top + mainROIBoundingBox.height);
    }

    // Calculate the main axis
    if ( imaqGetIntersection( boxEdge1Start, boxEdge1End, fitLine->start, fitLine->end, &origin ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }
    if ( imaqGetIntersection( boxEdge2Start, boxEdge2End, fitLine->start, fitLine->end, &mainAxisEnd ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }

    // Create the secondary search ROI  
    secondaryROI = imaqCreateROI();
    if ( secondaryROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }
    if ( (rectID = imaqAddRotatedRectContour2 (secondaryROI, secondaryRect)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, NULL );
        return FALSE;
    }
    if ( imaqSetContourColor( secondaryROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, NULL );
        return FALSE;
    }

    // Update the rake options for the secondary search area
    if( options ) {          
        optionsToUse.threshold = options->secondaryThreshold;
        optionsToUse.width = options->secondaryWidth;
        optionsToUse.steepness = options->secondarySteepness;
        optionsToUse.subsamplingRatio = options->secondarySubsamplingRatio;
    }
    
    // Locate the edges on the secondary axis.
    if ( (secondaryReportToProcess = imaqRake( image, secondaryROI, secondaryAxisDirection, IMAQ_FIRST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, NULL );
        return FALSE;
    }

    // Verify there is at least one edge
    if ( secondaryReportToProcess->numFirstEdges < 1 ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }

    // Find the closest first edge
    if ( imaqFindExtremeEdge( secondaryReportToProcess, TRUE, &closetFirstEdge ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }
    
    // Set the position of the secondary axis, adjusting the origin
    if ( imaqGetPerpendicularLine( fitLine->start, fitLine->end, closetFirstEdge, &secondaryAxisEnd, &origin, NULL) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }

    // Update the transform based on the mode
    transform->measurementSystem.origin = origin;   
    transform->measurementSystem.angle = (float)angle; 
    if ( imaqSetAxisOrienation( mainAxisDirection, secondaryAxisDirection, &transform->measurementSystem.axisOrientation ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }
    if ( mode == IMAQ_FIND_REFERENCE ) {
        transform->referenceSystem.origin = transform->measurementSystem.origin;    
        transform->referenceSystem.angle = transform->measurementSystem.angle;  
        transform->referenceSystem.axisOrientation = transform->measurementSystem.axisOrientation;  
    }
    
    // Merge the two ROIs into one (for the purpose of display and to
    // get the correct bounding box)
    if ( imaqCopyContour( mainROI, secondaryROI, rectID ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }
    
    // Update the bounding information to include both rectangles
    if ( imaqGetROIBoundingBox (mainROI, &mainROIBoundingBox) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }

    // Adjust the axis so it fits in the provided bounding box
    if ( imaqFitAxisReportToRect( mainROIBoundingBox, mainAxisDirection, secondaryAxisDirection, origin, &mainAxisEnd, &secondaryAxisEnd ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }

    // Set up the axis report (if requested)
    if ( report ) {
        report->origin = origin;
        report->mainAxisEnd = mainAxisEnd;
        report->secondaryAxisEnd = secondaryAxisEnd;
    }

    // Overlay the results on the image
    if ( imaqOverlayFindTransformRects( image, mainROI, mainReportToProcess, secondaryReportToProcess, origin, mainAxisEnd, secondaryAxisEnd, NULL, options ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, secondaryReportToProcess, NULL );
        return FALSE;
    }
    
    // Dispose of the allocated structures
    if ( imaqDispose(secondaryReportToProcess) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, secondaryROI, NULL );
        return FALSE;
    }
    if ( imaqDispose(secondaryROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, fitLine, NULL );
        return FALSE;
    }
    if ( imaqDispose(fitLine) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, mainReportToProcess, NULL );
        return FALSE;
    }
    if ( imaqDispose(mainReportToProcess) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, mainROI, NULL );
        return FALSE;
    }
    if ( imaqDispose(mainROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindTransformRectsErrorString, NULL );
        return FALSE;
    }

    return TRUE;

}

                   
//////////////////////////////////////////////////////////////////////////////
//  Count and Measure Objects
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  imaqCountObjects
//
//  Description:
//      Locates, counts, and measures objects in a rectangular search area.
//
//  Parameters:
//      image       -   The image that the function uses for object detection.
//      searchRect  -   The coordinate location of the rectangular search area 
//                      of the edge detection.
//      options     -   Describes how you want the function to detect objects 
//                      and  what information the function should overlay onto
//                      the image.
//      transform   -   An optional specification of the coordinate transform 
//                      for searchRect.
//      numObjects  -   Upon return, the size of the object report array
//
//  Return Value:
//      On success, an array of reports describing the objects. On error, NULL.
//
//////////////////////////////////////////////////////////////////////////////
ObjectReport* IMAQ_STDCALL imaqCountObjects(Image* image, RotatedRect searchRect, const CountObjectsOptions* options, const CoordinateTransform2* transform, int* numObjects) {

    ObjectReport* reportsToReturn = NULL;
    ROI* tempROI;
    ImageType imageType;
    Image* extractedImage;
    ContourID rectID;
    ObjectType objectType;
    float threshold, thresholdMin, thresholdMax;
    int rejectBorder, fillHoles, useMinSize, minSize, useMaxSize;
    int maxSize, xOffset, yOffset;
    Rect boundingBox;
    int i, numParticles, numParticlesToProcess;
    int* particlesToProcess = NULL;
    
    // Set up an ROI with the search rectangle
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, NULL );
        return NULL;
    }
    if ( (rectID = imaqAddRotatedRectContour2 (tempROI, searchRect)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqSetContourColor( tempROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, NULL );
        return NULL;
    }

    // If neccesary, transform the rotated rectangle
    if ( transform  ) {
        if ( imaqTransformROI2( tempROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, NULL );
            return NULL;
        }
    }

    // Set up the options, if available 
    if ( options ) {
        objectType   = options->type;
        threshold    = options->threshold;
        rejectBorder = options->rejectBorder;
        fillHoles    = options->fillHoles;
        useMinSize   = options->useMinSize;
        minSize      = options->minSize;
        useMaxSize   = options->useMaxSize;
        maxSize      = options->maxSize;
    }
    else {
        objectType   = DEFAULT_TYPE;
        threshold    = DEFAULT_OBJECT_THRESHOLD;
        rejectBorder = DEFAULT_REJECT_BORDER;
        fillHoles    = DEFAULT_FILL_HOLES;
        useMinSize   = DEFAULT_USE_MIN_SIZE;
        minSize      = DEFAULT_MIN_SIZE;
        useMaxSize   = DEFAULT_USE_MAX_SIZE;
        maxSize      = DEFAULT_MAX_SIZE;
    }

    // Get the bounding information for the ROI
    if ( imaqGetROIBoundingBox (tempROI, &boundingBox) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, NULL );
        return NULL;
    }

    // Extract the region of interest into a new image
    if ( imaqGetImageType (image, &imageType) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, NULL );
        return NULL;
    }
    if ( (extractedImage = imaqCreateImage(imageType, DEFAULT_BORDER_SIZE)) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqScale( extractedImage, image, 1, 1, IMAQ_SCALE_LARGER, boundingBox) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
        return NULL;
    }

    // Compute the offset of the extracted image
    xOffset = (boundingBox.left < 0) ? 0 : boundingBox.left;
    yOffset = (boundingBox.top  < 0) ? 0 : boundingBox.top;

    // If the rectangle is rotated OR the two coordinate systems have different 
    // angles, mask out the areas of the image that are not in the ROI
    if ( (searchRect.angle != 0) || ( transform && ( transform->referenceSystem.angle != transform->measurementSystem.angle)) ) {
        Image* maskImage;
        
        if ( (maskImage = imaqCreateImage(imageType, DEFAULT_BORDER_SIZE)) == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
            return NULL;
        }

        // Perform the mask
        if ( imaqROIToMask(maskImage, tempROI, 255, image, NULL) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, maskImage, NULL );
            return NULL;
        }
        
        // Extract the bounding region
        if ( imaqScale( maskImage, maskImage, 1, 1, IMAQ_SCALE_LARGER, boundingBox) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, maskImage, NULL );
            return NULL;
        }
        
        // Only combine the mask image with the extracted image so that
        // the extracted image contains the pixels of interest
        if ( (objectType == IMAQ_DARK_OBJECTS && !rejectBorder) || (objectType == IMAQ_BRIGHT_OBJECTS && rejectBorder) ) {
            if ( imaqInverse(maskImage, maskImage, NULL) == FALSE ) {
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, maskImage, NULL );
                return NULL;
            }
            if ( imaqOr(extractedImage, maskImage, extractedImage) == FALSE ) {
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, maskImage, NULL );
                return NULL;
            }
        }
        else {
            if ( imaqAnd(extractedImage, maskImage, extractedImage) == FALSE ) {
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, maskImage, NULL );
                return NULL;
            }
        }
        
        // Dispose of the mask image
        if ( imaqDispose(maskImage) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
            return NULL;
        }
    }     

    // Set the threshold range
    if ( objectType == IMAQ_DARK_OBJECTS ) {
        thresholdMin = -FLT_MAX;
        thresholdMax = threshold;
    }
    else {
        thresholdMin = threshold;
        thresholdMax = FLT_MAX;
    }
    
    // Threshold the image
    if ( imaqThreshold(extractedImage, extractedImage, thresholdMin, thresholdMax, TRUE, 1.0) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
        return NULL;
    }
    
    // Reject border objects and fill holes if the user requested it
    if ( rejectBorder ) {
        if ( imaqRejectBorder (extractedImage, extractedImage, TRUE) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
            return NULL;
        }
    }
    if ( fillHoles ) {
        if ( imaqFillHoles (extractedImage, extractedImage, TRUE) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
            return NULL;
        }
    }

    // Calculate the particle information
    if ( imaqCountParticles (extractedImage, TRUE, &numParticles) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
        return NULL;
    }
    
    if ( (particlesToProcess = malloc(sizeof(int) * numParticles)) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
        return NULL;
    }
    // Filter the particle reports for minimum and maximum particle sizes
    if ( useMinSize || useMaxSize ) {
        double curArea;
        numParticlesToProcess = 0;
        for (i = 0; i < numParticles; i++) {        
            if ( imaqMeasureParticle( extractedImage, i, FALSE, IMAQ_MT_AREA, &curArea ) == FALSE ) {
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
                free(particlesToProcess);
                return NULL;
            }
            if ((!useMinSize || (curArea >= (double)minSize)) &&
                (!useMaxSize || (curArea <= (double)maxSize))) {
                particlesToProcess[numParticlesToProcess] = i;
                numParticlesToProcess++;
            }
        }
    } else {
        // Process all particles.
        numParticlesToProcess = numParticles;
        for (i = 0; i < numParticles; i++) {
            particlesToProcess[i] = i;
        }
    }
    
    // Only proceed if we found at least one particle
    if ( numParticlesToProcess > 0 ) {
        
        double centerX, centerY, left, top, width, height, area, orientation;
        double numHoles;
        int i;

        // Allocate the object report array
        if ( (reportsToReturn = malloc(sizeof(ObjectReport) * numParticlesToProcess)) == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, NULL );
            free(particlesToProcess);
            return NULL;
        }

        for ( i = 0; i < numParticlesToProcess; i++ ) {
            // Calculate the coefficients
            int curParticle = particlesToProcess[i];
            if ( imaqMeasureParticle( extractedImage, curParticle, FALSE, IMAQ_MT_CENTER_OF_MASS_X, &centerX ) == FALSE ) {
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
                free(particlesToProcess);
                return NULL;
            }
            if ( imaqMeasureParticle( extractedImage, curParticle, FALSE, IMAQ_MT_CENTER_OF_MASS_Y, &centerY ) == FALSE ) {
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
                free(particlesToProcess);
                return NULL;
            }
            if ( imaqMeasureParticle (extractedImage, curParticle, FALSE, IMAQ_MT_BOUNDING_RECT_LEFT, &left ) == FALSE ) { 
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
                free(particlesToProcess);
                return NULL;
            }
            if ( imaqMeasureParticle (extractedImage, curParticle, FALSE, IMAQ_MT_BOUNDING_RECT_TOP, &top ) == FALSE ) { 
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
                free(particlesToProcess);
                return NULL;
            }
            if ( imaqMeasureParticle (extractedImage, curParticle, FALSE, IMAQ_MT_BOUNDING_RECT_WIDTH, &width ) == FALSE ) { 
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
                free(particlesToProcess);
                return NULL;
            }
            if ( imaqMeasureParticle (extractedImage, curParticle, FALSE, IMAQ_MT_BOUNDING_RECT_HEIGHT, &height ) == FALSE ) { 
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
                free(particlesToProcess);
                return NULL;
            }
            if ( imaqMeasureParticle (extractedImage, curParticle, FALSE, IMAQ_MT_AREA, &area ) == FALSE ) { 
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
                free(particlesToProcess);
                return NULL;
            }
            if ( imaqMeasureParticle (extractedImage, curParticle, FALSE, IMAQ_MT_ORIENTATION, &orientation ) == FALSE ) { 
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
                free(particlesToProcess);
                return NULL;
            }
            if ( imaqMeasureParticle (extractedImage, curParticle, FALSE, IMAQ_MT_NUMBER_OF_HOLES, &numHoles ) == FALSE ) { 
                imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
                free(particlesToProcess);
                return NULL;
            }
            // Set the report values
            reportsToReturn[i].center.x             = (float)(centerX + xOffset);
            reportsToReturn[i].center.y             = (float)(centerY + yOffset);
            reportsToReturn[i].boundingRect.left    = ROUND_TO_INT(left + xOffset);
            reportsToReturn[i].boundingRect.top     = ROUND_TO_INT(top + yOffset);
            reportsToReturn[i].boundingRect.width   = ROUND_TO_INT(width);
            reportsToReturn[i].boundingRect.height  = ROUND_TO_INT(height);
            reportsToReturn[i].area                 = (float)area;
            reportsToReturn[i].orientation          = (float)orientation;
            reportsToReturn[i].aspectRatio          = (float)(width / height);
            reportsToReturn[i].numHoles             = (int)numHoles;
        }
    }

    // Overlay the results
    if ( imaqOverlayCountObjectsResults( image, tempROI, reportsToReturn, numParticlesToProcess, options ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, tempROI, extractedImage, reportsToReturn, NULL );
        free(particlesToProcess);
        return NULL;
    }

    // Set the object count (if requested )
    if ( numObjects ) 
        *numObjects = numParticlesToProcess;

    // Dispose of allocated structures and images
    free(particlesToProcess);
    if ( imaqDispose(extractedImage) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, reportsToReturn, tempROI, NULL );
        return NULL;
    }
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqCountObjectsErrorString, reportsToReturn, NULL );
        return NULL;
    }   
    
    return reportsToReturn;

}


//////////////////////////////////////////////////////////////////////////////
//  Find Patterns
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  imaqFindPattern
//
//  Description:
//      Searches for a pattern image in a rectangular search area of the image.
//
//  Parameters:
//      image       -   The image that the function uses for pattern matching.
//      pattern     -   The pattern image which the function attempts to locate.
//      searchRect  -   The coordinate location of the rectangular search area 
//                      of the edge detection.
//      options     -   Describes how you want the function to detect patterns 
//                      and  what information the function should overlay onto
//                      the image.
//      transform   -   An optional specification of the coordinate transform 
//                      for searchRect.
//      numMatches  -   Upon return, the size of the pattern match array.
//
//  Return Value:
//      On success, an array of reports describing the matches. On error, NULL.
//
//////////////////////////////////////////////////////////////////////////////
PatternMatch* IMAQ_STDCALL imaqFindPattern(Image* image, Image* pattern, RotatedRect searchRect, const FindPatternOptions* options, const CoordinateTransform2* transform, int* numMatches) {

    PatternMatch* matchesToReturn = NULL;
    int matchingPresent;
    MatchPatternOptions matchOptions;
    int localNumMatches;
    ROI* tempROI;
    ContourID rectID;

    // Check that the pattern image has the pattern matching information we need
    if ( imaqIsVisionInfoPresent( pattern, IMAQ_PATTERN_MATCHING_INFO, &matchingPresent ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, NULL );
        return NULL;
    }

    // If there is no matching information, learn the pattern
    if ( !matchingPresent ) {
        LearningMode mode = IMAQ_LEARN_SHIFT_INFORMATION;
        if ( options && options->mode == IMAQ_MATCH_ROTATION_INVARIANT )
            mode = IMAQ_LEARN_ROTATION_INFORMATION;
        if ( imaqLearnPattern2( pattern, mode, NULL ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, NULL );
            return NULL;
        }
    }

    // Set up an ROI with the search rectangle for tranforming and display
    // purposes.
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, NULL );
        return NULL;
    }
    if ( (rectID = imaqAddRotatedRectContour2 (tempROI, searchRect)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqSetContourColor( tempROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, tempROI, NULL );
        return FALSE;
    }

    // If neccesary, transform the rotated rectangle
    if ( transform  ) {
        ContourInfo2* rectInfo;

        if ( imaqTransformROI2( tempROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, tempROI, NULL );
            return NULL;
        }
        rectInfo = imaqGetContourInfo2( tempROI, rectID );
        if( rectInfo ) {
             searchRect = *(rectInfo->structure.rotatedRect);
        }
        else {
            imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, tempROI, NULL );
            return NULL;        
        }
        // Dispose of the contour info
        if ( imaqDispose(rectInfo) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, tempROI, NULL );
            return NULL;
        }
    }

    // Set the match pattern options
    if ( options ) {
        matchOptions.mode = options->mode;
        matchOptions.minMatchScore = (float)options->minMatchScore;
        matchOptions.subpixelAccuracy = options->subpixelAccuracy;
        matchOptions.angleRanges = options->angleRanges;
        matchOptions.numRanges = options->numRanges;
        matchOptions.numMatchesRequested = options->numMatchesRequested;
    }
    else {
        matchOptions.mode = DEFAULT_MATCH_MODE;
        matchOptions.minMatchScore = DEFAULT_MIN_MATCH_SCORE;
        matchOptions.subpixelAccuracy = DEFAULT_SUBPIXEL;
        matchOptions.angleRanges = DEFAULT_ANGLE_RANGES;
        matchOptions.numRanges = DEFAULT_NUM_RANGES;
        matchOptions.numMatchesRequested = DEFAULT_NUM_MATCHES;
    }
    matchOptions.minContrast = DEFAULT_MIN_CONTRAST;
    matchOptions.matchFactor = DEFAULT_MATCH_FACTOR;

    // Look for the pattern
    if (( matchesToReturn = imaqMatchPattern2( image, pattern, &matchOptions, NULL, imaqMakeRectFromRotatedRect(searchRect), &localNumMatches )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, tempROI, NULL );
        return NULL;
    }
    
    // Overlay the results
    if ( imaqOverlayFindPatternResults( image, tempROI, matchesToReturn, localNumMatches, options ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, matchesToReturn, tempROI, NULL );
        return NULL;
    }
    
    // Set the number of matches, if needed
    if(numMatches)
        *numMatches = localNumMatches;

    // Dispose of the ROI
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindPatternErrorString, matchesToReturn, NULL );
        return NULL;
    }
    
    // Return the matches
    return matchesToReturn;

}


//////////////////////////////////////////////////////////////////////////////
//  Locate Edges
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  imaqFindCircularEdge
//
//  Description:
//      Locates a circular edge in an annular search area.
//
//  Parameters:
//      image       -   The image that the function uses for edge detection.
//      searchArea  -   The coordinate location of the annular search area 
//                      of the edge detection.
//      direction   -   The direction the function search for edges along the 
//                      search lines.
//      options     -   Describes how you want the function to detect edges and 
//                      what information the function should overlay onto the 
//                      image.
//      transform   -   An optional specification of the coordinate transform 
//                      for searchArea.
//
//  Return Value:
//      On success, a report describing the edge. On error, NULL.
//
//////////////////////////////////////////////////////////////////////////////
CircularEdgeReport* IMAQ_STDCALL imaqFindCircularEdge(Image* image, Annulus searchArea, SpokeDirection direction, const FindEdgeOptions* options, const CoordinateTransform2* transform) {

    CircularEdgeReport* reportToReturn = NULL;
    ROI* tempROI;
    ContourID rectID;
    SpokeReport* reportToProcess;
    SpokeOptions optionsToUse;
    PointFloat center = {0, 0};
    double radius     = 0;
    double roundness  = 0;
    BestCircle fitCircle;
    int i;
    int validCircle = FALSE;
    
    // Create an ROI, place the annulus contour inside it and set the contour color
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, NULL );
        return NULL;
    }
    if ( (rectID = imaqAddAnnulusContour (tempROI, searchArea)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqSetContourColor( tempROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, tempROI, NULL );
        return NULL;
    }

    // If neccesary, transform the annulus.
    if ( transform  ) {
        if ( imaqTransformROI2( tempROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, tempROI, NULL );
            return NULL;
        }
    }

    // Use the supplied rake options (if available) or use default options
    if( options ) {
        optionsToUse.threshold = options->threshold;
        optionsToUse.width = options->width;
        optionsToUse.steepness = options->steepness;
        optionsToUse.subsamplingRatio = options->subsamplingRatio;
    }
    else {
        optionsToUse.threshold = DEFAULT_THRESHOLD;
        optionsToUse.width = DEFAULT_WIDTH;
        optionsToUse.steepness = DEFAULT_STEEPNESS;
        optionsToUse.subsamplingRatio = DEFAULT_SUBSAMPLING_RATIO;
    }

    // Set the remaining options
    optionsToUse.subpixelType = IMAQ_CUBIC_SPLINE;
    optionsToUse.subpixelDivisions = DEFAULT_SUBPIXEL_DIVISIONS;

    // Calculate the edge locations
    if ( (reportToProcess = imaqSpoke( image, tempROI, direction, IMAQ_FIRST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, tempROI, NULL );
        return NULL;
    }   

    // If there were at least two edges, calculate a line along the edges
    if ( reportToProcess->numFirstEdges >= 3 ) {
        if ( imaqFitCircle( reportToProcess->firstEdges, reportToProcess->numFirstEdges, &fitCircle) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, tempROI, reportToProcess, NULL );
            return NULL;
        }
        validCircle = TRUE;
        center      = fitCircle.center;
        radius      = fitCircle.radius;
        roundness   = fitCircle.error;
    }

    // Set up the report to return
    if ( (reportToReturn = malloc(sizeof(CircularEdgeReport))) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, tempROI, reportToProcess, NULL );
        return NULL;
    }
    reportToReturn->center = center;
    reportToReturn->radius = radius;
    reportToReturn->roundness = roundness;
    if(validCircle) {
        if ( (reportToReturn->coordinates = malloc(sizeof(PointFloat) * reportToProcess->numFirstEdges )) == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, tempROI, reportToProcess, reportToReturn, NULL );
            return NULL;
        }
        reportToReturn->numCoordinates = reportToProcess->numFirstEdges;
        for ( i = 0; i < reportToProcess->numFirstEdges; i++ ) {
            reportToReturn->coordinates[i] = reportToProcess->firstEdges[i];
        }
    }
    else {
        reportToReturn->coordinates = NULL;
        reportToReturn->numCoordinates = 0;
    }

    // Overlay the results
    if ( imaqOverlayFindCircularEdgeResults( image, tempROI, reportToProcess, reportToReturn, options ) == FALSE ) {
        imaqDisposeCircularEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, tempROI, reportToProcess, NULL );
        return NULL;
    }

    // Dispose of allocated structures
    if ( imaqDispose(reportToProcess) == FALSE ) {
        imaqDisposeCircularEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqDisposeCircularEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindCircularEdgeErrorString, NULL );
        return NULL;
    }

    return reportToReturn;

}

//////////////////////////////////////////////////////////////////////////////
//
//  imaqFindConcentricEdge
//
//  Description:
//      Locates a straight edge in an annular search area.
//
//  Parameters:
//      image       -   The image that the function uses for edge detection.
//      searchArea  -   The coordinate location of the annular search area 
//                      of the edge detection.
//      direction   -   The direction the function search for edges along the 
//                      search lines.
//      options     -   Describes how you want the function to detect edges and 
//                      what information the function should overlay onto the 
//                      image.
//      transform   -   An optional specification of the coordinate transform 
//                      for searchArea.
//
//  Return Value:
//      On success, a report describing the edge. On error, NULL.
//
//////////////////////////////////////////////////////////////////////////////
StraightEdgeReport* IMAQ_STDCALL imaqFindConcentricEdge(Image* image, Annulus searchArea, ConcentricRakeDirection direction, const FindEdgeOptions* options, const CoordinateTransform2* transform) {

    StraightEdgeReport* reportToReturn = NULL;
    ROI* tempROI;
    ContourID rectID;
    ConcentricRakeReport* reportToProcess;
    RakeOptions optionsToUse;
    PointFloat edgeStart = {0, 0};
    PointFloat edgeEnd   = {0, 0};
    double straightness = 0;
    BestLine* fitLine = NULL;
    FitLineOptions fitLineOptions;
    int i;

    // Create an ROI, place the annulus contour inside it and set the contour color
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, NULL );
        return NULL;
    }
    if ( (rectID = imaqAddAnnulusContour (tempROI, searchArea)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqSetContourColor( tempROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, NULL );
        return NULL;
    }

    // If neccesary, transform the annulus.
    if ( transform  ) {
        if ( imaqTransformROI2( tempROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, NULL );
            return NULL;
        }
    }

    // Use the supplied rake options (if available) or use default options
    if( options ) {
        optionsToUse.threshold = options->threshold;
        optionsToUse.width = options->width;
        optionsToUse.steepness = options->steepness;
        optionsToUse.subsamplingRatio = (int)options->subsamplingRatio;
    }
    else {
        optionsToUse.threshold = DEFAULT_THRESHOLD;
        optionsToUse.width = DEFAULT_WIDTH;
        optionsToUse.steepness = DEFAULT_STEEPNESS;
        optionsToUse.subsamplingRatio = DEFAULT_SUBSAMPLING_RATIO;
    }

    // Set the remaining options
    optionsToUse.subpixelType = IMAQ_CUBIC_SPLINE;
    optionsToUse.subpixelDivisions = DEFAULT_SUBPIXEL_DIVISIONS;

    // Calculate the edge locations
    if ( (reportToProcess = imaqConcentricRake( image, tempROI, direction, IMAQ_FIRST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, NULL );
        return NULL;
    }

    // If there were at least two edges, calculate a line along the edges
    if ( reportToProcess->numFirstEdges >= 2 ) {
    fitLineOptions.minScore       = DEFAULT_MIN_LINE_SCORE;
    fitLineOptions.pixelRadius    = DEFAULT_PIXEL_RADIUS;
    fitLineOptions.numRefinements = DEFAULT_REFINEMENTS;
        if ( (fitLine = imaqFitLine( reportToProcess->firstEdges, reportToProcess->numFirstEdges, &fitLineOptions )) == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, reportToProcess, NULL );
            return NULL;
        }
        // Ensure that we got the score we wanted before proceeding
        if( fitLine->valid ) {
            edgeStart    = fitLine->start;
            edgeEnd      = fitLine->end;
            straightness = fitLine->error;
        }
    }

    // Set up the report to return
    if ( (reportToReturn = malloc(sizeof(StraightEdgeReport))) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, reportToProcess, fitLine, NULL );
        return NULL;
    }
    reportToReturn->start = edgeStart;
    reportToReturn->end = edgeEnd;
    reportToReturn->straightness = straightness;
    if(fitLine) {
        if ( (reportToReturn->coordinates = malloc(sizeof(PointFloat) * fitLine->numPointsUsed)) == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, reportToProcess, fitLine, reportToReturn, NULL );
            return NULL;
        }
        reportToReturn->numCoordinates = fitLine->numPointsUsed;
        for ( i = 0; i < fitLine->numPointsUsed; i++ ) {
            reportToReturn->coordinates[i] = reportToProcess->firstEdges[fitLine->pointsUsed[i]];
        }
    }
    else {
        reportToReturn->coordinates = NULL;
        reportToReturn->numCoordinates = 0;
    }

    // Overlay the results
    if ( imaqOverlayFindConcentricEdgeResults( image, tempROI, reportToProcess, reportToReturn, direction, options ) == FALSE ) {
        imaqDisposeStraightEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, reportToProcess, fitLine, NULL );
        return NULL;
    }

    // Dispose of allocated structures
    if ( imaqDispose(fitLine) == FALSE ) {
        imaqDisposeStraightEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, reportToProcess, NULL );
        return NULL;
    }
    if ( imaqDispose(reportToProcess) == FALSE ) {
        imaqDisposeStraightEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqDisposeStraightEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindConcentricEdgeErrorString, NULL );
        return NULL;
    }

    return reportToReturn;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqFindEdge
//
//  Description:
//      Locates a straight edge in a rectangular search area.
//
//  Parameters:
//      image       -   The image that the function uses for edge detection.
//      searchRect  -   The coordinate location of the rectangular search area 
//                      of the edge detection.
//      direction   -   The direction the function search for edges along the 
//                      search lines.
//      options     -   Describes how you want the function to detect edges and 
//                      what information the function should overlay onto the 
//                      image.
//      transform   -   An optional specification of the coordinate transform 
//                      for searchRect.
//
//  Return Value:
//      On success, a report describing the edge. On error, NULL.
//
//////////////////////////////////////////////////////////////////////////////
StraightEdgeReport* IMAQ_STDCALL imaqFindEdge(Image* image, RotatedRect searchRect, RakeDirection direction, const FindEdgeOptions* options, const CoordinateTransform2* transform) {

    StraightEdgeReport* reportToReturn = NULL;
    ROI* tempROI;
    ContourID rectID;
    RakeOptions optionsToUse;
    RakeReport* reportToProcess;
    PointFloat edgeStart = {0, 0};
    PointFloat edgeEnd   = {0, 0};
    double straightness = 0;
    BestLine* fitLine = NULL;
    FitLineOptions fitLineOptions;
    int i;
    
    // Create an ROI, place the rotated rect contour inside it and set the contour color
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, NULL );
        return NULL;
    }
    if ( (rectID = imaqAddRotatedRectContour2 (tempROI, searchRect)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqSetContourColor( tempROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, NULL );
        return NULL;
    }

    // If neccesary, transform the rectangle.
    if ( transform  ) {
        if ( imaqTransformROI2( tempROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, NULL );
            return NULL;
        }
    }

    // Use the supplied rake options (if available) or use default options
    if( options ) {
        optionsToUse.threshold = options->threshold;
        optionsToUse.width = options->width;
        optionsToUse.steepness = options->steepness;
        optionsToUse.subsamplingRatio = (int)options->subsamplingRatio;
    }
    else {
        optionsToUse.threshold = DEFAULT_THRESHOLD;
        optionsToUse.width = DEFAULT_WIDTH;
        optionsToUse.steepness = DEFAULT_STEEPNESS;
        optionsToUse.subsamplingRatio = DEFAULT_SUBSAMPLING_RATIO;
    }

    // Set the remaining options
    optionsToUse.subpixelType = IMAQ_CUBIC_SPLINE;
    optionsToUse.subpixelDivisions = DEFAULT_SUBPIXEL_DIVISIONS;

    // Calculate the edge locations
    if ( (reportToProcess = imaqRake( image, tempROI, direction, IMAQ_FIRST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, NULL );
        return NULL;
    }

    // If there were at least two edges, calculate a line along the edges
    if ( reportToProcess->numFirstEdges >= 2 ) {
        fitLineOptions.minScore       = DEFAULT_MIN_LINE_SCORE;
        fitLineOptions.pixelRadius    = DEFAULT_PIXEL_RADIUS;
        fitLineOptions.numRefinements = DEFAULT_REFINEMENTS;
        if ( (fitLine = imaqFitLine( reportToProcess->firstEdges, reportToProcess->numFirstEdges, &fitLineOptions )) == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, reportToProcess, NULL );
            return NULL;
        }
        // Ensure that we got the score we wanted before proceeding
        if( fitLine->valid ) {
            // Set the edge line to extend to touch all search lines
            if ( imaqGetIntersection( reportToProcess->rakeLines[0].start, reportToProcess->rakeLines[0].end, fitLine->start, fitLine->end, &edgeStart) == FALSE ) {
                imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, reportToProcess, fitLine, NULL );
                return NULL;
            }
            if ( imaqGetIntersection( reportToProcess->rakeLines[reportToProcess->numRakeLines - 1].start, reportToProcess->rakeLines[reportToProcess->numRakeLines - 1].end, fitLine->start, fitLine->end, &edgeEnd) == FALSE ) {
                imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, reportToProcess, fitLine, NULL );
                return NULL;
            }
            straightness = fitLine->error;
        }
    }

    // Set up the report to return
    if ( (reportToReturn = malloc(sizeof(StraightEdgeReport))) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, reportToProcess, fitLine, NULL );
        return NULL;
    }
    reportToReturn->start = edgeStart;
    reportToReturn->end = edgeEnd;
    reportToReturn->straightness = straightness;
    if(fitLine) {
        if ( (reportToReturn->coordinates = malloc(sizeof(PointFloat) * fitLine->numPointsUsed)) == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, reportToProcess, fitLine, reportToReturn, NULL );
            return NULL;
        }
        reportToReturn->numCoordinates = fitLine->numPointsUsed;
        for ( i = 0; i < fitLine->numPointsUsed; i++ ) {
            reportToReturn->coordinates[i] = reportToProcess->firstEdges[fitLine->pointsUsed[i]];
        }
    }
    else {
        reportToReturn->coordinates = NULL;
        reportToReturn->numCoordinates = 0;
    }
    
    // Overlay the results
    if ( imaqOverlayFindEdgeResults( image, tempROI, reportToProcess, reportToReturn, options ) == FALSE ) {
        imaqDisposeStraightEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, reportToProcess, fitLine, NULL );
        return NULL;
    }

    // Dispose of allocated structures
    if ( imaqDispose(fitLine) == FALSE ) {
        imaqDisposeStraightEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, reportToProcess, NULL );
        return NULL;
    }
    if ( imaqDispose(reportToProcess) == FALSE ) {
        imaqDisposeStraightEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqDisposeStraightEdgeReport(reportToReturn);
        reportToReturn = NULL;      
        imaqPrepareForExit( imaqGetLastError(), imaqFindEdgeErrorString, NULL );
        return NULL;
    }

    return reportToReturn;

}

//////////////////////////////////////////////////////////////////////////////
//  Measure Distances
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  imaqClampMax
//
//  Description:
//      Measures a distance from the sides of the search area towards the 
//      center of the search area.
//
//  Parameters:
//      image       -   The image that the function uses for distance 
//                      measurement.
//      searchRect  -   The coordinate location of the rectangular search area 
//                      of the distance measurement.
//      direction   -   The direction the function search for edges along the 
//                      search lines.
//      distance    -   Upon return, the distance measured between the two 
//                      parallel hit-lines. This parameter is required and 
//                      cannot be NULL.
//      options     -   Describes how you want the function to detect edges and 
//                      what information the function should overlay onto the 
//                      image.
//      transform   -   An optional specification of the coordinate transform 
//                      for searchRect.
//      firstEdge   -   Upon return, the coordinate location of the first edge 
//                      used to measure the distance.
//      lastEdge    -   Upon return, the coordinate location of the last edge 
//                      used to measure the distance.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqClampMax(Image* image, RotatedRect searchRect, RakeDirection direction, float* distance, const FindEdgeOptions* options, const CoordinateTransform2* transform, PointFloat* firstEdge, PointFloat* lastEdge) {

    ROI* tempROI;
    ContourID rectID;
    RakeReport* reportToProcess;
    RakeOptions optionsToUse;
    LineFloat firstPerpendicularLine, lastPerpendicularLine;
    PointFloat localFirstEdge, localLastEdge;

    // Verify that the distance is not NULL
    if( distance == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqClampMaxErrorString, NULL );
        return FALSE;
    }

    // Create an ROI, place the rotated rect contour inside it and set the contour color
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMaxErrorString, NULL );
        return FALSE;
    }
    if ( (rectID = imaqAddRotatedRectContour2 (tempROI, searchRect)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMaxErrorString, tempROI, NULL );
        return FALSE;
    }
    if ( imaqSetContourColor( tempROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMaxErrorString, tempROI, NULL );
        return FALSE;
    }

    // If neccesary, transform the rectangle.
    if ( transform  ) {
        if ( imaqTransformROI2( tempROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqClampMaxErrorString, tempROI, NULL );
            return FALSE;
        }
    }

    // Use the supplied rake options (if available) or use default options
    if( options ) {
        optionsToUse.threshold = options->threshold;
        optionsToUse.width = options->width;
        optionsToUse.steepness = options->steepness;
        optionsToUse.subsamplingRatio = (int)options->subsamplingRatio;
    }
    else {
        optionsToUse.threshold = DEFAULT_THRESHOLD;
        optionsToUse.width = DEFAULT_WIDTH;
        optionsToUse.steepness = DEFAULT_STEEPNESS;
        optionsToUse.subsamplingRatio = DEFAULT_SUBSAMPLING_RATIO;
    }

    // Set the remaining options
    optionsToUse.subpixelType = IMAQ_CUBIC_SPLINE;
    optionsToUse.subpixelDivisions = DEFAULT_SUBPIXEL_DIVISIONS;

    // Calculate the edge locations
    if ( (reportToProcess = imaqRake( image, tempROI, direction, IMAQ_FIRST_AND_LAST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMaxErrorString, tempROI, NULL );
        return FALSE;
    }

    // Use the rake report to calculate the maximum distance
    if ( imaqMeasureMaxDistance( reportToProcess, &firstPerpendicularLine, &lastPerpendicularLine, distance, &localFirstEdge, &localLastEdge ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMaxErrorString, tempROI, reportToProcess, NULL );
        return FALSE;
    }
    
    // Display the results
    if ( imaqOverlayClampResults( image, tempROI, reportToProcess, NULL, &firstPerpendicularLine, &lastPerpendicularLine, localFirstEdge, localLastEdge, TRUE, options ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMaxErrorString, tempROI, reportToProcess, NULL );
        return FALSE;
    }

    // Set the start and end points (if requested)
	if (firstEdge){
		firstEdge[0] = localFirstEdge;
		firstEdge[1] = firstPerpendicularLine.start;
	}
	if (lastEdge){
		lastEdge[0] = localLastEdge;
		lastEdge[1] = lastPerpendicularLine.start;
	}

    // Dispose of the rake report
    if ( imaqDispose(reportToProcess) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMaxErrorString, tempROI, NULL );
        return FALSE;
    }

    // Dispose of the ROI
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMaxErrorString, NULL );
        return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqClampMin
//
//  Description:
//      Measures a distance from the center of the search area towards the 
//      sides of the search area.
//
//  Parameters:
//      image       -   The image that the function uses for distance 
//                      measurement.
//      searchRect  -   The coordinate location of the rectangular search area 
//                      of the distance measurement.
//      direction   -   The direction the function search for edges along the 
//                      search lines.
//      distance    -   Upon return, the distance measured between the two 
//                      parallel hit-lines. This parameter is required and 
//                      cannot be NULL.
//      options     -   Describes how you want the function to detect edges and 
//                      what information the function should overlay onto the 
//                      image.
//      transform   -   An optional specification of the coordinate transform 
//                      for searchRect.
//      firstEdge   -   Upon return, the coordinate location of the first edge 
//                      used to measure the distance.
//      lastEdge    -   Upon return, the coordinate location of the last edge 
//                      used to measure the distance.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqClampMin(Image* image, RotatedRect searchRect, RakeDirection direction, float* distance, const FindEdgeOptions* options, const CoordinateTransform2* transform, PointFloat* firstEdge, PointFloat* lastEdge) {

    ROI* totalSearchRectROI;
    RotatedRect firstSearchRect;
    ROI* firstSearchRectROI;
    RotatedRect secondSearchRect;
    ROI* secondSearchRectROI;
    ContourID rectID;
    RakeReport* firstReport;
    RakeReport* secondReport;
    RakeOptions optionsToUse;
    LineFloat firstPerpendicularLine, lastPerpendicularLine;
    PointFloat localFirstEdge, localLastEdge;
    ContourInfo2* rectInfo;
	RakeDirection oppositeDirection = IMAQ_RIGHT_TO_LEFT;

    // Verify that the distance is not NULL
    if( distance == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqClampMinErrorString, NULL );
        return FALSE;
    }

    // Create an ROI, place the rotated rect contour inside it and set the contour color
    totalSearchRectROI = imaqCreateROI();
    if ( totalSearchRectROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, NULL );
        return FALSE;
    }
    if ( (rectID = imaqAddRotatedRectContour2 (totalSearchRectROI, searchRect)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, NULL );
        return FALSE;
    }
    if ( imaqSetContourColor( totalSearchRectROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, NULL );
        return FALSE;
    }

    // If neccesary, transform the rectangle.
    if ( transform  ) {
        if ( imaqTransformROI2( totalSearchRectROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, NULL );
            return FALSE;
        }
    }

    // Get the rect from the ROI
    rectInfo = imaqGetContourInfo2( totalSearchRectROI, rectID );
    if( rectInfo ) {
         searchRect = *(rectInfo->structure.rotatedRect);
    }
    else {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, NULL );
        return FALSE;       
    }

    // Dispose of the contour info
    if ( imaqDispose(rectInfo) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, NULL );
        return FALSE;
    }

    // Separate the rect into two rectangles
    if ( imaqSplitRotatedRectangle( searchRect, direction, &firstSearchRect, &secondSearchRect ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, NULL );
        return FALSE;
    }
    
    // Place each of the half rectangles into a ROI
    firstSearchRectROI = imaqCreateROI();
    if ( firstSearchRectROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, NULL );
        return FALSE;
    }
    if ( imaqAddRotatedRectContour2 (firstSearchRectROI, firstSearchRect) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, firstSearchRectROI, NULL );
        return FALSE;
    }
    secondSearchRectROI = imaqCreateROI();
    if ( secondSearchRectROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, firstSearchRectROI, NULL );
        return FALSE;
    }
    if ( imaqAddRotatedRectContour2 (secondSearchRectROI, secondSearchRect) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, firstSearchRectROI, secondSearchRectROI, NULL );
        return FALSE;
    }

    // Use the supplied rake options (if available) or use default options
    if( options ) {
        optionsToUse.threshold = options->threshold;
        optionsToUse.width = options->width;
        optionsToUse.steepness = options->steepness;
        optionsToUse.subsamplingRatio = (int)options->subsamplingRatio;
    }
    else {
        optionsToUse.threshold = DEFAULT_THRESHOLD;
        optionsToUse.width = DEFAULT_WIDTH;
        optionsToUse.steepness = DEFAULT_STEEPNESS;
        optionsToUse.subsamplingRatio = DEFAULT_SUBSAMPLING_RATIO;
    }

    // Set the remaining options
    optionsToUse.subpixelType = IMAQ_CUBIC_SPLINE;
    optionsToUse.subpixelDivisions = DEFAULT_SUBPIXEL_DIVISIONS;

    
    // Calculate the edge locations for the second half rect 
    if ( (secondReport = imaqRake( image, secondSearchRectROI, direction, IMAQ_FIRST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, firstSearchRectROI, secondSearchRectROI, NULL );
        return FALSE;
    }

    // Find the opposite direction, then calculate the edge locations for the 
    // second half rect 
    switch ( direction ) {
        case IMAQ_LEFT_TO_RIGHT:
            oppositeDirection = IMAQ_RIGHT_TO_LEFT;
            break;
        case IMAQ_RIGHT_TO_LEFT:
            oppositeDirection = IMAQ_LEFT_TO_RIGHT;
            break;
        case IMAQ_TOP_TO_BOTTOM:
            oppositeDirection = IMAQ_BOTTOM_TO_TOP;
            break;
        case IMAQ_BOTTOM_TO_TOP:
            oppositeDirection = IMAQ_TOP_TO_BOTTOM;
            break;

    }   
    if ( (firstReport = imaqRake( image, firstSearchRectROI, oppositeDirection, IMAQ_FIRST, &optionsToUse )) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, firstSearchRectROI, secondSearchRectROI, secondReport, NULL );
        return FALSE;
    }

    // Use the rake report to calculate the minimum distance
    if ( imaqMeasureMinDistance( firstReport, secondReport, &firstPerpendicularLine, &lastPerpendicularLine, distance, &localFirstEdge, &localLastEdge ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, firstSearchRectROI, secondSearchRectROI, firstReport, secondReport, NULL );
        return FALSE;
    }
    
    // Display the results
    if ( imaqOverlayClampResults( image, totalSearchRectROI, firstReport, secondReport, &firstPerpendicularLine, &lastPerpendicularLine, localFirstEdge, localLastEdge, FALSE, options ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, firstSearchRectROI, secondSearchRectROI, firstReport, secondReport, NULL );
        return FALSE;
    }

    // Set the start and end points (if requested)
    if(firstEdge)
        *firstEdge = localFirstEdge;
    if(lastEdge)
        *lastEdge = localLastEdge;

    // Dispose of the rake reports
    if ( imaqDispose(secondReport) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, firstSearchRectROI, secondSearchRectROI, firstReport, NULL );
        return FALSE;
    }
    if ( imaqDispose(firstReport) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, firstSearchRectROI, secondSearchRectROI, NULL );
        return FALSE;
    }
    
    // Dispose of the ROIs
    if ( imaqDispose(secondSearchRectROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, firstSearchRectROI, totalSearchRectROI, NULL );
        return FALSE;
    }
    if ( imaqDispose(firstSearchRectROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, totalSearchRectROI, NULL );
        return FALSE;
    }
    if ( imaqDispose(totalSearchRectROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqClampMinErrorString, NULL );
        return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//  Measure Intensities functions
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  imaqLightMeterLine
//
//  Description:
//      Measures the pixel intensities on a line of an image.
//
//  Parameters:
//      image           -   The image that the function uses for intensity
//                          measurement.
//      start           -   The coordinate location of the start of the line.
//      end             -   The coordinate location of the end of the line.
//      showMeasurement -   Set this parameter to TRUE to overlay the location 
//                          of the intensity measurement on the image. Set this 
//                          parameter to FALSE to leave the image unmodified.
//      transform       -   An optional specification of the coordinate 
//                          transform for the line. 
//
//  Return Value:
//      LineProfile*    -   The intensity information for the line
//
//////////////////////////////////////////////////////////////////////////////
LineProfile* IMAQ_STDCALL imaqLightMeterLine( Image* image, Point start, Point end, int showMeasurement, const CoordinateTransform2* transform ) {

    Point startToUse;
    Point endToUse;
    LineProfile* profileToReturn = NULL;

    // If neccesary, transform the line.
    if ( transform  ) {
        ROI* tempROI;
        ContourID lineID;
        ContourInfo2* lineInfo;

        tempROI = imaqCreateROI();
        if ( tempROI == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterLineErrorString, NULL );
            return NULL;
        }
        if ( (lineID = imaqAddLineContour (tempROI, start, end)) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterLineErrorString, tempROI, NULL );
            return NULL;
        }
        if ( imaqTransformROI2( tempROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterLineErrorString, tempROI, NULL );
            return NULL;
        }
        lineInfo = imaqGetContourInfo2( tempROI, lineID );
        if( lineInfo ) {
             startToUse = (lineInfo->structure.line->start);
             endToUse   = (lineInfo->structure.line->end);
        }
        else {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterLineErrorString, tempROI, NULL );
            return NULL;        
        }
        // Dispose of the temp ROI and the contour info
        if ( imaqDispose(lineInfo) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterLineErrorString, tempROI, NULL );
            return NULL;
        }
        if ( imaqDispose(tempROI) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterLineErrorString, NULL );
            return NULL;
        }
    }
    else { 
        startToUse  = start;
        endToUse    = end;
    }
    
    // Calculate the line profile
    if( (profileToReturn = imaqLineProfile (image, startToUse, endToUse)) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterLineErrorString, NULL );
        return NULL;
    }
    
    // If neccesary, overlay the location of the measurement
    if( showMeasurement ) {
        if ( imaqOverlayLine( image, startToUse, endToUse, &IMAQ_RGB_GREEN, defaultGroup) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterLineErrorString, profileToReturn, NULL );
            return NULL;
        }
    }
    
    return profileToReturn;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqLightMeterPoint
//
//  Description:
//      Measures the pixel intensities in a 3 x 3 pixel neighborhood centered 
//      on a point of an image.
//
//  Parameters:
//      image           -   The image that the function uses for intensity
//                          measurement.
//      point           -   The coordinate location of the intensity 
//                          measurement. The intensity measurement is made in a
//                          3 x 3 block centered on the point.
//      showMeasurement -   Set this parameter to TRUE to overlay the location 
//                          of the intensity measurement on the image. Set this 
//                          parameter to FALSE to leave the image unmodified.
//      intensity       -   Upon return, the average intensity of the pixels in
//                          a 3 x 3 neighborhood centered on the point. This 
//                          parameter is required and cannot be NULL.
//      transform       -   An optional specification of the coordinate 
//                          transform for point. 
//
//  Return Value:
//      Error code
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqLightMeterPoint( Image* image, Point point, int showMeasurement, float* intensity, const CoordinateTransform2* transform ) {

    Point pointToUse;
    Rect  rectToUse;
    ImageType imageType;
    int i, j;
    int columns;
    int rows;
    void* pixelArray;
    float localIntensity = 0;
    
    // Verify that the intensity is not NULL
    if( intensity == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqLightMeterPointErrorString, NULL );
        return FALSE;
    }

    // Verify that the image is of a type the function supports
    if ( imaqGetImageType (image, &imageType) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, NULL );
        return FALSE;
    }
    if ( imageType != IMAQ_IMAGE_U8 && imageType != IMAQ_IMAGE_I16 && imageType != IMAQ_IMAGE_SGL ) { 
        imaqPrepareForExit( ERR_INVALID_IMAGE_TYPE, imaqLightMeterPointErrorString, NULL );
        return FALSE;
    }

    // If neccesary, transform the point.
    if ( transform  ) {
        ROI* tempROI;
        ContourID pointID;
        ContourInfo2* pointInfo;

        tempROI = imaqCreateROI();
        if ( tempROI == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, NULL );
            return FALSE;
        }
        if ( (pointID = imaqAddPointContour (tempROI, point)) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, tempROI, NULL );
            return FALSE;
        }
        if ( imaqTransformROI2( tempROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, tempROI, NULL );
            return FALSE;
        }
        pointInfo = imaqGetContourInfo2( tempROI, pointID );
        if( pointInfo ) {
             pointToUse = *(pointInfo->structure.point);
        }
        else {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, tempROI, NULL );
            return FALSE;       
        }
        // Dispose of the temp ROI and the contour info
        if ( imaqDispose(pointInfo) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, tempROI, NULL );
            return FALSE;
        }
        if ( imaqDispose(tempROI) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, NULL );
            return FALSE;
        }
    }
    else 
        pointToUse = point;

    // Set up the rect to be 3x3 centered on the pixel      
    rectToUse = imaqMakeRect( pointToUse.y - 1, pointToUse.x - 1, MEASURE_POINT_WIDTH, MEASURE_POINT_HEIGHT );          
        
    // Get the pixel information around the point
    pixelArray = imaqImageToArray( image, rectToUse, &columns, &rows );
    if( pixelArray == NULL ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, NULL );
            return FALSE;
    }

    // Processing of the pixel information depends on the image type
    switch ( imageType ) {
        default:
        case IMAQ_IMAGE_U8: {
            unsigned char* pixelPtr = pixelArray;
            for ( i = 0; i < columns; i++ ) {
                for ( j = 0; j < rows; j++ ) {
                    localIntensity += *pixelPtr++;
                }
            }
            break;
        }
        case IMAQ_IMAGE_I16: {
            short* pixelPtr = pixelArray;
            for ( i = 0; i < columns; i++ ) {
                for ( j = 0; j < rows; j++ ) {
                    localIntensity += *pixelPtr++;
                }
            }
            break;
        }
        case IMAQ_IMAGE_SGL: {
            float* pixelPtr = pixelArray;
            for ( i = 0; i < columns; i++ ) {
                for ( j = 0; j < rows; j++ ) {
                    localIntensity += *pixelPtr++;
                }
            }
            break;
        }
    }

    // Set the intensity, being sure not to divide by zero 
    *intensity = localIntensity / ( columns * rows > 0 ? columns * rows : 1 );
    
    // If neccesary, overlay the location of the measurement
    if( showMeasurement ) {
        if ( imaqOverlayRect( image, rectToUse, &IMAQ_RGB_GREEN, IMAQ_DRAW_VALUE, defaultGroup) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, pixelArray, NULL );
            return FALSE;
        }
    }
    
    // Dispose of the pixel array
    if ( imaqDispose( pixelArray ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterPointErrorString, NULL );
        return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqLightMeterRect
//
//  Description:
//      Measures the pixel intensities inside a rectangular region of an image.
//
//  Parameters:
//      image           -   The image that the function uses for intensity
//                          measurement.
//      rect            -   The coordinate location of the rectangular area of 
//                          the intensity measurement.
//      showMeasurement -   Set this parameter to TRUE to overlay the location 
//                          of the intensity measurement on the image. Set this 
//                          parameter to FALSE to leave the image unmodified.
//      transform       -   An optional specification of the coordinate 
//                          transform for the line. 
//
//  Return Value:
//      HistogramReport* -  The intensity information for the rectangular region.
//
//////////////////////////////////////////////////////////////////////////////
HistogramReport* IMAQ_STDCALL imaqLightMeterRect( Image* image, RotatedRect rect, int showMeasurement, const CoordinateTransform2* transform ) {

    ROI* tempROI;
    ContourID rectID;
    Image* maskImage;
    HistogramReport* reportToReturn;

    // Create an ROI, place the rotated rect contour inside it and set the contour color
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, NULL );
        return NULL;
    }
    if ( (rectID = imaqAddRotatedRectContour2 (tempROI, rect)) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqSetContourColor( tempROI, rectID, &IMAQ_RGB_GREEN ) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, tempROI, NULL );
        return NULL;
    }

    // If neccesary, transform the rectangle.
    if ( transform  ) {
        if ( imaqTransformROI2( tempROI, &transform->referenceSystem, &transform->measurementSystem ) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, tempROI, NULL );
            return NULL;
        }
    }

    // Create the mask image
    if ( (maskImage = imaqCreateImage(IMAQ_IMAGE_U8, DEFAULT_BORDER_SIZE)) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, tempROI, NULL );
        return NULL;
    }
    if ( imaqROIToMask (maskImage, tempROI, 255, NULL, NULL) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, tempROI, NULL );
        return NULL;
    }
    
    // Calculate the histogram of the rectangular region
    if ( (reportToReturn = imaqHistogram (image, 256, 0.0, 0.0, maskImage)) == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, tempROI, maskImage, NULL );
        return NULL;
    }
    
    // Dispose of the mask image
    if ( imaqDispose(maskImage) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, reportToReturn, tempROI, NULL );
        return NULL;
    }

    // If neccesary, overlay the location of the measurement
    if( showMeasurement ) {
        if ( imaqOverlayROI( image, tempROI, IMAQ_POINT_AS_CROSS, NULL, defaultGroup) == FALSE ) {
            imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, reportToReturn, tempROI, NULL );
            return NULL;
        }
    }
    
    // Dispose of the ROI
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqLightMeterRectErrorString, reportToReturn, NULL );
        return NULL;
    }
    
    return reportToReturn;

}


//////////////////////////////////////////////////////////////////////////////
// Select Region Of Interest functions
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  imaqSelectAnnulus
//
//  Description:
//      Displays the image in a modal window and allows the user to draw an 
//      annulus on it.  After the user draws the annulus, the function hides 
//      the window.
//
//  Parameters:
//      image   -   The image from which the user selects an annulus.
//      annulus -   Upon return, this parameter specifies the coordinates of 
//                  the annulus chosen by the user. This parameter is required 
//                  and cannot be NULL.
//      options -   Describes how a function presents the annulus constructor 
//                  window.
//      okay    -   Upon return, this parameter is TRUE if the user pressed the 
//                  "OK" button to end the selection of an annulus. Otherwise, 
//                  this parameter is FALSE. Set this parameter to NULL if you 
//                  do not need this information.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqSelectAnnulus( const Image* image, Annulus* annulus, const ConstructROIOptions* options, int* okay ) {

    int localOkay;
    int contourCount;
    ROI* tempROI = NULL;
    ToolWindowOptions toolOptions;
    ConstructROIOptions constructionOptions;
    ContourInfo2* annulusInfo = NULL;
    ContourID annulusID;
    Annulus noSelection = { -1, -1, -1, -1, -1, -1 };

    // Verify that the annulus is not NULL
    if( annulus == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqSelectAnnulusErrorString, NULL );
        return FALSE;
    }

    // Create the ROI
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectAnnulusErrorString, NULL );
        return FALSE;
    }

    // Set the tool options
    toolOptions.showSelectionTool = TRUE;
    toolOptions.showZoomTool = TRUE;
    toolOptions.showPointTool = FALSE;
    toolOptions.showLineTool = FALSE;
    toolOptions.showRectangleTool = FALSE;
    toolOptions.showOvalTool = FALSE;
    toolOptions.showPolygonTool = FALSE;
    toolOptions.showClosedFreehandTool = FALSE;
    toolOptions.showPolyLineTool = FALSE;
    toolOptions.showFreehandTool = FALSE;
    toolOptions.showAnnulusTool = TRUE;
    toolOptions.showRotatedRectangleTool = FALSE;
    toolOptions.showPanTool = TRUE;

    // Set the construction options to user values (if supplied) or default
    // values
    if( options ) {
        constructionOptions = *options; 
    }
    else {
        constructionOptions.windowNumber = IMAQ_MODAL_DIALOG;
        constructionOptions.windowTitle = defaultAnnulusWindowTitle;
        constructionOptions.type = IMAQ_PALETTE_GRAY;
        constructionOptions.palette = NULL;
        constructionOptions.numColors = 0;
    }
    
    // Call the ROI constructor
    if( imaqConstructROI( image, tempROI, IMAQ_ANNULUS_TOOL, &toolOptions, &constructionOptions, &localOkay ) == FALSE) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectAnnulusErrorString, tempROI, NULL);
        return FALSE;
    }

    // Get the contour information (if the user pressed okay)
    if( localOkay ) {
        if ( (contourCount = imaqGetContourCount( tempROI )) == -1 ) {
            imaqPrepareForExit( imaqGetLastError(), imaqSelectAnnulusErrorString, tempROI, NULL);
            return FALSE;       
        }
        if (contourCount > 0 ) {
            if ( (annulusID = imaqGetContour (tempROI, 0)) == 0 ) {
                imaqPrepareForExit( imaqGetLastError(), imaqSelectAnnulusErrorString, tempROI, NULL);
                return FALSE;       
            }
            annulusInfo = imaqGetContourInfo2( tempROI, annulusID );
            if( annulusInfo ) {
                 *annulus = *(annulusInfo->structure.annulus);
            }
            else {
                imaqPrepareForExit( imaqGetLastError(), imaqSelectAnnulusErrorString, tempROI, NULL);
                return FALSE;       
            }
        }
        else {
            // The user did not select an annulus
            *annulus = noSelection;
        }
    }
    else 
        // The user did not select an annulus
        *annulus = noSelection;
    
    // Return the exit condition of the constructor (if requested)
    if ( okay ) 
        *okay = localOkay;

    // Dispose of the temp ROI and the contour info
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectAnnulusErrorString, annulusInfo, NULL );
        return FALSE;
    }
    if ( imaqDispose(annulusInfo) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectAnnulusErrorString, NULL );
        return FALSE;
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqSelectLine
//
//  Description:
//      Displays the image in a modal window and allows the user to draw a 
//      line on it.  After the user draws the line, the function hides 
//      the window.
//
//  Parameters:
//      image   -   The image from which the user selects a line.
//      start   -   Upon return, this parameter specifies the coordinates of 
//                  the start of the line chosen by the user. This parameter is 
//                  required and cannot be NULL.
//      end     -   Upon return, this parameter specifies the coordinates of 
//                  the end of the line chosen by the user. This parameter is 
//                  required and cannot be NULL.
//      options -   Describes how a function presents the line constructor 
//                  window.
//      okay    -   Upon return, this parameter is TRUE if the user pressed the 
//                  "OK" button to end the selection of an line. Otherwise, 
//                  this parameter is FALSE. Set this parameter to NULL if you 
//                  do not need this information.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqSelectLine( const Image* image, Point* start, Point* end, const ConstructROIOptions* options, int* okay ) {

    int localOkay;
    int contourCount;
    ROI* tempROI = NULL;
    ToolWindowOptions toolOptions;
    ConstructROIOptions constructionOptions;
    ContourInfo2* lineInfo = NULL;
    ContourID lineID;
    Point noSelection = { -1, -1 };

    // Verify that the start and end points are not NULL
    if( start == NULL || end == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqSelectLineErrorString, NULL );
        return FALSE;
    }

    // Create the ROI
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectLineErrorString, NULL );
        return FALSE;
    }

    // Set the tool options
    toolOptions.showSelectionTool = TRUE;
    toolOptions.showZoomTool = TRUE;
    toolOptions.showPointTool = FALSE;
    toolOptions.showLineTool = TRUE;
    toolOptions.showRectangleTool = FALSE;
    toolOptions.showOvalTool = FALSE;
    toolOptions.showPolygonTool = FALSE;
    toolOptions.showClosedFreehandTool = FALSE;
    toolOptions.showPolyLineTool = FALSE;
    toolOptions.showFreehandTool = FALSE;
    toolOptions.showAnnulusTool = FALSE;
    toolOptions.showRotatedRectangleTool = FALSE;
    toolOptions.showPanTool = TRUE;

    // Set the construction options to user values (if supplied) or default
    // values
    if( options ) {
        constructionOptions = *options; 
    }
    else {
        constructionOptions.windowNumber = IMAQ_MODAL_DIALOG;
        constructionOptions.windowTitle = defaultLineWindowTitle;
        constructionOptions.type = IMAQ_PALETTE_GRAY;
        constructionOptions.palette = NULL;
        constructionOptions.numColors = 0;
    }
    
    // Call the ROI constructor
    if( imaqConstructROI( image, tempROI, IMAQ_LINE_TOOL, &toolOptions, &constructionOptions, &localOkay ) == FALSE) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectLineErrorString, tempROI, NULL);
        return FALSE;
    }

    // Get the contour information (if the user pressed okay)
    if( localOkay ) {
        if ( (contourCount = imaqGetContourCount( tempROI )) == -1 ) {
            imaqPrepareForExit( imaqGetLastError(), imaqSelectLineErrorString, tempROI, NULL);
            return FALSE;       
        }
        if (contourCount > 0 ) {
            if ( (lineID = imaqGetContour (tempROI, 0)) == 0 ) {
                imaqPrepareForExit( imaqGetLastError(), imaqSelectLineErrorString, tempROI, NULL);
                return FALSE;       
            }
            lineInfo = imaqGetContourInfo2( tempROI, lineID );
            if( lineInfo ) {
                 *start = lineInfo->structure.line->start;
                 *end   = lineInfo->structure.line->end;
            }
            else {
                imaqPrepareForExit( imaqGetLastError(), imaqSelectLineErrorString, tempROI, NULL);
                return FALSE;       
            }
        }
        else {
            // The user did not select a line
            *start = noSelection;
            *end = noSelection;
        }
    }
    else {
        // The user did not select a line
            *start = noSelection;
            *end = noSelection;
    }
    
    // Return the exit condition of the constructor (if requested)
    if ( okay ) 
        *okay = localOkay;

    // Dispose of the temp ROI and the contour info
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectLineErrorString, lineInfo, NULL );
        return FALSE;
    }
    if ( imaqDispose(lineInfo) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectLineErrorString, NULL );
        return FALSE;
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqSelectPoint
//
//  Description:
//      Displays the image in a modal window and allows the user to draw a 
//      point on it.  After the user draws the line, the function hides 
//      the window.
//
//  Parameters:
//      image   -   The image from which the user selects a line.
//      point   -   Upon return, this parameter specifies the coordinates of 
//                  the point chosen by the user. This parameter is required 
//                  and cannot be NULL.
//      options -   Describes how a function presents the point constructor 
//                  window.
//      okay    -   Upon return, this parameter is TRUE if the user pressed the 
//                  "OK" button to end the selection of an point. Otherwise, 
//                  this parameter is FALSE. Set this parameter to NULL if you 
//                  do not need this information.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqSelectPoint( const Image* image, Point* point, const ConstructROIOptions* options, int* okay ) {

    int localOkay;
    int contourCount;
    ROI* tempROI = NULL;
    ToolWindowOptions toolOptions;
    ConstructROIOptions constructionOptions;
    ContourInfo2* pointInfo = NULL;
    ContourID pointID;
    Point noSelection = { -1, -1 };

    // Verify that the point is not NULL
    if( point == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqSelectPointErrorString, NULL );
        return FALSE;
    }

    // Create the ROI
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectPointErrorString, NULL );
        return FALSE;
    }

    // Set the tool options
    toolOptions.showSelectionTool = TRUE;
    toolOptions.showZoomTool = TRUE;
    toolOptions.showPointTool = TRUE;
    toolOptions.showLineTool = FALSE;
    toolOptions.showRectangleTool = FALSE;
    toolOptions.showOvalTool = FALSE;
    toolOptions.showPolygonTool = FALSE;
    toolOptions.showClosedFreehandTool = FALSE;
    toolOptions.showPolyLineTool = FALSE;
    toolOptions.showFreehandTool = FALSE;
    toolOptions.showAnnulusTool = FALSE;
    toolOptions.showRotatedRectangleTool = FALSE;
    toolOptions.showPanTool = TRUE;

    // Set the construction options to user values (if supplied) or default
    // values
    if( options ) {
        constructionOptions = *options; 
    }
    else {
        constructionOptions.windowNumber = IMAQ_MODAL_DIALOG;
        constructionOptions.windowTitle = defaultPointWindowTitle;
        constructionOptions.type = IMAQ_PALETTE_GRAY;
        constructionOptions.palette = NULL;
        constructionOptions.numColors = 0;
    }
    
    // Call the ROI constructor
    if( imaqConstructROI( image, tempROI, IMAQ_POINT_TOOL, &toolOptions, &constructionOptions, &localOkay ) == FALSE) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectPointErrorString, tempROI, NULL);
        return FALSE;
    }

    // Get the contour information (if the user pressed okay)
    if( localOkay ) {
        if ( (contourCount = imaqGetContourCount( tempROI )) == -1 ) {
            imaqPrepareForExit( imaqGetLastError(), imaqSelectPointErrorString, tempROI, NULL);
            return FALSE;       
        }
        if (contourCount > 0 ) {
            if ( (pointID = imaqGetContour (tempROI, 0)) == 0 ) {
                imaqPrepareForExit( imaqGetLastError(), imaqSelectPointErrorString, tempROI, NULL);
                return FALSE;       
            }
            pointInfo = imaqGetContourInfo2( tempROI, pointID );
            if( pointInfo ) {
                 *point = *(pointInfo->structure.point);
            }
            else {
                imaqPrepareForExit( imaqGetLastError(), imaqSelectPointErrorString, tempROI, NULL);
                return FALSE;       
            }
        }
        else {
            // The user did not select a point
            *point = noSelection;
        }
    }
    else {
        // The user did not select a point
            *point = noSelection;
    }
    
    // Return the exit condition of the constructor (if requested)
    if ( okay ) 
        *okay = localOkay;

    // Dispose of the temp ROI and the contour info
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectPointErrorString, pointInfo, NULL );
        return FALSE;
    }
    if ( imaqDispose(pointInfo) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectPointErrorString, NULL );
        return FALSE;
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqSelectRect
//
//  Description:
//      Displays the image in a modal window and allows the user to draw a 
//      rectangle on it.  After the user draws the rectangle, the function 
//      hides the window.
//
//  Parameters:
//      image   -   The image from which the user selects a line.
//      rect    -   Upon return, this parameter specifies the coordinates of 
//                  the rect chosen by the user. This parameter is required 
//                  and cannot be NULL.
//      options -   Describes how a function presents the point constructor 
//                  window.
//      okay    -   Upon return, this parameter is TRUE if the user pressed the 
//                  "OK" button to end the selection of an point. Otherwise, 
//                  this parameter is FALSE. Set this parameter to NULL if you 
//                  do not need this information.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqSelectRect( const Image* image, RotatedRect* rect, const ConstructROIOptions* options, int* okay ) {

    int localOkay;
    int contourCount;
    ROI* tempROI = NULL;
    ToolWindowOptions toolOptions;
    ConstructROIOptions constructionOptions;
    ContourInfo2* rectInfo = NULL;
    ContourID rectID;
    RotatedRect noSelection = { -1, -1, -1, -1, -1 };

    // Verify that the rect is not NULL
    if( rect == NULL ) {
        imaqPrepareForExit( ERR_NULL_POINTER, imaqSelectRectErrorString, NULL );
        return FALSE;
    }

    // Create the ROI
    tempROI = imaqCreateROI();
    if ( tempROI == NULL ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectRectErrorString, NULL );
        return FALSE;
    }

    // Set the tool options
    toolOptions.showSelectionTool = TRUE;
    toolOptions.showZoomTool = TRUE;
    toolOptions.showPointTool = FALSE;
    toolOptions.showLineTool = FALSE;
    toolOptions.showRectangleTool = TRUE;
    toolOptions.showOvalTool = FALSE;
    toolOptions.showPolygonTool = FALSE;
    toolOptions.showClosedFreehandTool = FALSE;
    toolOptions.showPolyLineTool = FALSE;
    toolOptions.showFreehandTool = FALSE;
    toolOptions.showAnnulusTool = FALSE;
    toolOptions.showRotatedRectangleTool = TRUE;
    toolOptions.showPanTool = TRUE;

    // Set the construction options to user values (if supplied) or default
    // values
    if( options ) {
        constructionOptions = *options; 
    }
    else {
        constructionOptions.windowNumber = IMAQ_MODAL_DIALOG;
        constructionOptions.windowTitle = defaultRectWindowTitle;
        constructionOptions.type = IMAQ_PALETTE_GRAY;
        constructionOptions.palette = NULL;
        constructionOptions.numColors = 0;
    }
    
    // Call the ROI constructor
    if( imaqConstructROI( image, tempROI, IMAQ_RECTANGLE_TOOL, &toolOptions, &constructionOptions, &localOkay ) == FALSE) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectRectErrorString, tempROI, NULL);
        return FALSE;
    }

    // Get the contour information (if the user pressed okay)
    if( localOkay ) {
        if ( (contourCount = imaqGetContourCount( tempROI )) == -1 ) {
            imaqPrepareForExit( imaqGetLastError(), imaqSelectRectErrorString, tempROI, NULL);
            return FALSE;       
        }
        if (contourCount > 0 ) {
            if ( (rectID = imaqGetContour (tempROI, 0)) == 0 ) {
                imaqPrepareForExit( imaqGetLastError(), imaqSelectRectErrorString, tempROI, NULL);
                return FALSE;       
            }
            rectInfo = imaqGetContourInfo2( tempROI, rectID );
            if( rectInfo ) {
                if(rectInfo->type == IMAQ_RECT) {
                    *rect = imaqMakeRotatedRectFromRect(*(rectInfo->structure.rect));
                }
                else {
                    *rect = *(rectInfo->structure.rotatedRect);
                }
            }
            else {
                imaqPrepareForExit( imaqGetLastError(), imaqSelectRectErrorString, tempROI, NULL);
                return FALSE;       
            }
        }
        else {
            // The user did not select a rectangle
            *rect = noSelection;
        }
    }
    else {
        // The user did not select a rectangle
            *rect = noSelection;
    }
    
    // Return the exit condition of the constructor (if requested)
    if ( okay ) 
        *okay = localOkay;

    // Dispose of the temp ROI and the contour info
    if ( imaqDispose(tempROI) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectRectErrorString, rectInfo, NULL );
        return FALSE;
    }
    if ( imaqDispose(rectInfo) == FALSE ) {
        imaqPrepareForExit( imaqGetLastError(), imaqSelectRectErrorString, NULL );
        return FALSE;
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//  Utility functions
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  imaqDisposeCircularEdgeReport
//
//  Description:
//      Disposes of the memory associated with a CircularEdgeReport
//
//  Parameters:
//      report  -   The report to dispose
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqDisposeCircularEdgeReport( CircularEdgeReport* report ) {

    // Dispose of the array in the report, then the report
    if(report) {
        free(report->coordinates);
        free(report);
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqDisposeObjectReport
//
//  Description:
//      Disposes of the memory associated with an ObjectReport
//
//  Parameters:
//      report  -   The report to dispose
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqDisposeObjectReport( ObjectReport* report ) {

    // Dispose of the report array
    if(report) {
        free(report);
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqDisposeStraightEdgeReport
//
//  Description:
//      Disposes of the memory associated with a StraightEdgeReport
//
//  Parameters:
//      report  -   The report to dispose
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int IMAQ_STDCALL imaqDisposeStraightEdgeReport( StraightEdgeReport* report ) {

    // Dispose of the array in the report, then the report
    if(report) {
        free(report->coordinates);
        free(report);
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqFindExtremeEdge
//
//  Description:
//      Finds the edge that is either closest or farthest from the start
//      of a set of rake lines.
//
//  Parameters:
//      report          -   The rake report to process.
//      findClosestEdge -   The direction of the main axis.
//                          If FALSE, report must contain both first and last
//                          edge information
//      extremeEdge     -   Upon return, the extreme edge.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqFindExtremeEdge( const RakeReport* report, int findClosestEdge, PointFloat* edge ) {

    unsigned int i;
    float tempDistance, extremeDistance;
        
    // Verify that the rake report and PointFloat pointers are not NULL
    if( report == NULL || edge == NULL) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Determine if the function should process the first edges or the last edges array
    if ( findClosestEdge ) {
        if( report->numFirstEdges > 0 ) {
            // First, intialize the values with the first edge in the array
            if( imaqGetDistance( report->rakeLines[(report->linesWithEdges[0])].start, report->firstEdges[0], &extremeDistance ) == FALSE ) {
                return FALSE;
            }
            *edge = report->firstEdges[0];

            // Then find the edge that is closest to the boundary of the search area
            for ( i = 1; i < report->numFirstEdges; i++ ) {
                if( imaqGetDistance( report->rakeLines[(report->linesWithEdges[i])].start, report->firstEdges[i], &tempDistance ) == FALSE ) {
                    return FALSE;
                }
                if ( tempDistance < extremeDistance) {
                    extremeDistance = tempDistance;
                    *edge = report->firstEdges[i];
                }
            }
        }
    }
    else {
        if( report->numLastEdges > 0 ) {
        
            // First, intialize the value with the first edge in the array
            if( imaqGetDistance( report->rakeLines[(report->linesWithEdges[0])].end, report->lastEdges[0], &extremeDistance ) == FALSE ) {
                return FALSE;
            }
            *edge = report->lastEdges[0];

            // Then find the edge that are closest to the boundary of the search area
            for ( i = 1; i < report->numLastEdges; i++ ) {
                if( imaqGetDistance( report->rakeLines[(report->linesWithEdges[i])].end, report->lastEdges[i], &tempDistance ) == FALSE ) {
                    return FALSE;
                }
                if ( tempDistance < extremeDistance) {
                    extremeDistance = tempDistance;
                    *edge = report->lastEdges[i];
                }
            }
        }
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqFitAxisReportToRect
//
//  Description:
//      Fits the axis of an axis report to end at the extended boundaries of a
//      rectangle
//
//  Parameters:
//      rect                    -   The bounding rect
//      mainAxisDirection       -   The direction of the main axis
//      secondaryAxisDirection  -   The direction of the secondary axis
//      origin                  -   The origin of the axis.
//      mainAxisEnd             -   The end of the main axis, which the function
//                                  fits to the rect
//      secondaryAxisEnd        -   The end of the secondary axis, which the
//                                  function fits to the rect
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqFitAxisReportToRect( Rect rect, RakeDirection mainAxisDirection, RakeDirection secondaryAxisDirection, PointFloat origin, PointFloat* mainAxisEnd, PointFloat* secondaryAxisEnd ) {

    PointFloat topLeftCorner, topRightCorner, bottomLeftCorner, bottomRightCorner;

    // Verify that the axis end pointers are not NULL
    if( mainAxisEnd == NULL || secondaryAxisEnd == NULL) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Set the corners of the rect
    topLeftCorner.x = (float)rect.left;
    topLeftCorner.y = (float)rect.top;
    topRightCorner.x = (float)(rect.left + rect.width);
    topRightCorner.y = (float)rect.top;
    bottomLeftCorner.x = (float)rect.left;
    bottomLeftCorner.y = (float)(rect.top + rect.height);
    bottomRightCorner.x = (float)(rect.left + rect.width);
    bottomRightCorner.y = (float)(rect.top + rect.height);

    // Fit the main axis
    switch (secondaryAxisDirection) {
        case IMAQ_LEFT_TO_RIGHT:
            // The main axis ends at the right side of the bounding rect
            if( imaqGetIntersection( origin, *mainAxisEnd, topRightCorner, bottomRightCorner, mainAxisEnd ) == FALSE )
                return FALSE;
            break;
        case IMAQ_RIGHT_TO_LEFT:
            // The main axis ends at the left side of the bounding rect
            if( imaqGetIntersection( origin, *mainAxisEnd, topLeftCorner, bottomLeftCorner, mainAxisEnd ) == FALSE )
                return FALSE;
            break;
        case IMAQ_TOP_TO_BOTTOM:
            // The main axis ends at the bottom of the bounding rect
            if( imaqGetIntersection( origin, *mainAxisEnd, bottomLeftCorner, bottomRightCorner, mainAxisEnd ) == FALSE )
                return FALSE;
            break;
        case IMAQ_BOTTOM_TO_TOP:
            // The main axis ends at the top of the bounding rect
            if( imaqGetIntersection( origin, *mainAxisEnd, topLeftCorner, topRightCorner, mainAxisEnd ) == FALSE )
                return FALSE;
            break;
        default:
            imaqSetError( ERR_INVALID_RAKE_DIRECTION, NULL );
            return FALSE;
    }

    // Fit the secondary axis
    switch (mainAxisDirection) {
        case IMAQ_LEFT_TO_RIGHT:
            // The secondary axis ends at the right side of the bounding rect
            if( imaqGetIntersection( origin, *secondaryAxisEnd, topRightCorner, bottomRightCorner, secondaryAxisEnd ) == FALSE )
                return FALSE;
            break;
        case IMAQ_RIGHT_TO_LEFT:
            // The secondary axis ends at the left side of the bounding rect
            if( imaqGetIntersection( origin, *secondaryAxisEnd, topLeftCorner, bottomLeftCorner, secondaryAxisEnd ) == FALSE )
                return FALSE;
            break;
        case IMAQ_TOP_TO_BOTTOM:
            // The secondary axis ends at the bottom of the bounding rect
            if( imaqGetIntersection( origin, *secondaryAxisEnd, bottomLeftCorner, bottomRightCorner, secondaryAxisEnd ) == FALSE )
                return FALSE;
            break;
        case IMAQ_BOTTOM_TO_TOP:
            // The secondary axis ends at the top of the bounding rect
            if( imaqGetIntersection( origin, *secondaryAxisEnd, topLeftCorner, topRightCorner, secondaryAxisEnd ) == FALSE )
                return FALSE;
            break;
        default:
            imaqSetError( ERR_INVALID_RAKE_DIRECTION, NULL );
            return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqMakePointFromPointFloat
//
//  Description:
//      Converts a PointFloat into a Point
//
//  Parameters:
//      pointFloat - The point to convert.
//
//  Return Value:
//      The converted point.
//
//////////////////////////////////////////////////////////////////////////////
Point imaqMakePointFromPointFloat( PointFloat pointFloat ) {

    Point pointToReturn;
    
    pointToReturn.x = ROUND_TO_INT(pointFloat.x);
    pointToReturn.y = ROUND_TO_INT(pointFloat.y);

    return pointToReturn;
    
}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqMeasureMaxDistance
//
//  Description:
//      Measures the maximum distance between edges of a rake report
//
//  Parameters:
//      reportToProcess         -   The rake report. Must be generated with 
//                                  process equal to IMAQ_FIRST_AND_LAST or 
//                                  IMAQ_ALL. 
//      firstPerpendicularLine  -   Upon return, a line perpendicular to the 
//                                  first edge point. 
//                                  This is an optional parameter.
//      lastPerpendicularLine   -   Upon return, a line perpendicular to the 
//                                  last edge point.
//      distance                -   Upon return, the distance between the two
//                                  perpendicular lines.
//      firstEdge               -   Upon return, the location of the first edge.
//      lastEdge                -   Upon return, the location of the last edge.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int  imaqMeasureMaxDistance( const RakeReport* reportToProcess, LineFloat* firstPerpendicularLine, LineFloat* lastPerpendicularLine, float* distance, PointFloat* firstEdge, PointFloat* lastEdge ) {

    // Verify that the rake report pointer, the line pointers and the edge pointers are not NULL
    if( reportToProcess == NULL || firstPerpendicularLine == NULL || lastPerpendicularLine  == NULL || distance == NULL || firstEdge == NULL || lastEdge == NULL ) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Intialize all returned data to zero
    firstPerpendicularLine->start.x = 0;
    firstPerpendicularLine->start.y = 0;
    firstPerpendicularLine->end.x = 0;
    firstPerpendicularLine->end.y = 0;
    lastPerpendicularLine->start.x = 0;
    lastPerpendicularLine->start.y = 0;
    lastPerpendicularLine->end.x = 0;
    lastPerpendicularLine->end.y = 0;
    *distance = 0; 
    firstEdge->x = 0;
    firstEdge->y = 0;
    lastEdge->x = 0;
    lastEdge->y = 0;

    // Only proceed if there was at least one rake line
    if( reportToProcess->numRakeLines > 0  ) {
        // Locate the two extreme edges
        if ( imaqFindExtremeEdge( reportToProcess, TRUE, firstEdge ) == FALSE )
            return FALSE;
        if ( imaqFindExtremeEdge( reportToProcess, FALSE, lastEdge ) == FALSE )
            return FALSE;
    
        // Find the edge lines by find perpendicular lines from the edges to the
        // rake lines.
        if ( imaqGetPerpendicularLine( reportToProcess->rakeLines[0].start, reportToProcess->rakeLines[0].end, *firstEdge, NULL, &firstPerpendicularLine->start, NULL) == FALSE ) {
            return FALSE;
        }
        if ( imaqGetPerpendicularLine( reportToProcess->rakeLines[reportToProcess->numRakeLines - 1].start, reportToProcess->rakeLines[reportToProcess->numRakeLines - 1].end, *firstEdge, NULL, &firstPerpendicularLine->end, NULL) == FALSE ) {
            return FALSE;
        }
        if ( imaqGetPerpendicularLine( reportToProcess->rakeLines[0].start, reportToProcess->rakeLines[0].end, *lastEdge, NULL, &lastPerpendicularLine->start, NULL) == FALSE ) {
            return FALSE;
        }
        if ( imaqGetPerpendicularLine( reportToProcess->rakeLines[reportToProcess->numRakeLines - 1].start, reportToProcess->rakeLines[reportToProcess->numRakeLines - 1].end, *lastEdge, NULL, &lastPerpendicularLine->end, NULL) == FALSE ) {
            return FALSE;
        }

        // Compute the distance
        if( imaqGetDistance( firstPerpendicularLine->start, lastPerpendicularLine->start, distance ) == FALSE )
            return FALSE;
    }
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqMeasureMinDistance
//
//  Description:
//      Measures the minimum distance between edges of two rake reports
//
//  Parameters:
//      firstReport             -   The first rake report.
//      secondReport            -   The second rake report.
//      firstPerpendicularLine  -   Upon return, a line perpendicular to the 
//                                  first edge point. 
//      lastPerpendicularLine   -   Upon return, a line perpendicular to the 
//                                  last edge point.
//      distance                -   Upon return, the distance between the two
//                                  perpendicular lines.
//      firstEdge               -   Upon return, the location of the first edge.
//      lastEdge                -   Upon return, the location of the last edge.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int  imaqMeasureMinDistance( const RakeReport* firstReport, const RakeReport* secondReport, LineFloat* firstPerpendicularLine, LineFloat* lastPerpendicularLine, float* distance, PointFloat* firstEdge, PointFloat* lastEdge ) {

    // Verify that the rake report, the distance, the edge pointers  and the perpendicular line pointers are not NULL
    if( firstReport == NULL || secondReport == NULL || firstPerpendicularLine == NULL || lastPerpendicularLine == NULL || distance == NULL || firstEdge == NULL || lastEdge == NULL ) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Intialize all returned data to zero
    firstPerpendicularLine->start.x = 0;
    firstPerpendicularLine->start.y = 0;
    firstPerpendicularLine->end.x = 0;
    firstPerpendicularLine->end.y = 0;
    lastPerpendicularLine->start.x = 0;
    lastPerpendicularLine->start.y = 0;
    lastPerpendicularLine->end.x = 0;
    lastPerpendicularLine->end.y = 0;
    *distance = 0; 
    firstEdge->x = 0;
    firstEdge->y = 0;
    lastEdge->x = 0;
    lastEdge->y = 0;

    // Only proceed if there was at least one rake line in each rake report
    if( firstReport->numRakeLines > 0 && secondReport->numRakeLines > 0  ) {
        // Find the edges that are closest to the boundaries of each search area
        if ( imaqFindExtremeEdge( firstReport, TRUE, firstEdge ) == FALSE )
            return FALSE;
        if ( imaqFindExtremeEdge( secondReport, TRUE, lastEdge ) == FALSE )
            return FALSE;

        // Find the edge lines by find perpendicular lines from the edges to the
        // rake lines.
        if ( imaqGetPerpendicularLine( firstReport->rakeLines[0].start, firstReport->rakeLines[0].end, *firstEdge, NULL, &(firstPerpendicularLine->start), NULL) == FALSE ) {
            return FALSE;
        }
        if ( imaqGetPerpendicularLine( firstReport->rakeLines[firstReport->numRakeLines - 1].start, firstReport->rakeLines[firstReport->numRakeLines - 1].end, *firstEdge, NULL, &(firstPerpendicularLine->end), NULL) == FALSE ) {
            return FALSE;
        }
        if ( imaqGetPerpendicularLine( secondReport->rakeLines[0].start, secondReport->rakeLines[0].end, *lastEdge, NULL, &(lastPerpendicularLine->start), NULL) == FALSE ) {
            return FALSE;
        }
        if ( imaqGetPerpendicularLine( secondReport->rakeLines[secondReport->numRakeLines - 1].start, secondReport->rakeLines[secondReport->numRakeLines -1].end, *lastEdge, NULL, &(lastPerpendicularLine->end), NULL) == FALSE ) {
            return FALSE;
        }

        // Compute the distance
        if( imaqGetDistance( firstPerpendicularLine->start, lastPerpendicularLine->start, distance ) == FALSE ) {
            return FALSE;
        }
    }    
    
    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayArrow
//
//  Description:
//      Overlays an arrows
//
//  Parameters:
//      image           -   The image on which the function overlays the arrow.
//      pointAtArrow    -   The point where the function overlays the arrow.
//      pointOnLine     -   A point along the line of the arrow. Should be a
//                          different point then pointAtArrow
//      color           -   The color of the line.
//      reserved        -   Reserved. Set to NULL.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayArrow(Image* image, Point pointAtArrow, Point pointOnLine, const RGBValue* color, void* reserved) {

    float dx, dy, slope, R, A;
    Point arrow[3];
    int calcArrows = FALSE;

    // Anchor the arrow on the point
    arrow[0] = pointAtArrow;

    // Calculate the slope of the line
    dx = (float)(pointAtArrow.x - pointOnLine.x);
    dy = (float)(pointAtArrow.y - pointOnLine.y);
    if (dx == 0)
        if((dy == 0)) {
            // If the two points are equal, assume the slope is zero
            slope = 0;
            calcArrows = TRUE;
        }   
        else {
            // Handle the cases where the line is perpendicular to the X axis
            if (dy < 0) {
                arrow[1].x = pointAtArrow.x - 2;
                arrow[1].y = pointAtArrow.y + 4;
                arrow[2].x = pointAtArrow.x + 2;
                arrow[2].y = pointAtArrow.y + 4;
            }
            else {
                arrow[1].x = pointAtArrow.x - 2;
                arrow[1].y = pointAtArrow.y - 4;
                arrow[2].x = pointAtArrow.x + 2;
                arrow[2].y = pointAtArrow.y - 4;
            }
        }
    else
    {
        slope = (dy/dx);
        calcArrows = TRUE;
    }

    // Calculate the location of the arrow (if needed)
    if (calcArrows) {
        R = (float)(atan(slope) * 180 / IMAQ_PI);
        A = (dx<0)?(R + 180):(dy<0)?(R+360):R;
        R = (float)(A* IMAQ_PI / 180);

        arrow[1].x = pointAtArrow.x - ROUND_TO_INT(4.0 * cos(R - 0.35));
        arrow[1].y = pointAtArrow.y - ROUND_TO_INT(4.0 * sin(R - 0.35));
        arrow[2].x = pointAtArrow.x - ROUND_TO_INT(4.0 * cos(0.35 + R));
        arrow[2].y = pointAtArrow.y - ROUND_TO_INT(4.0 * sin(0.35 + R));
    }

	if (imaqOverlayClosedContour(image, arrow, 3, color, IMAQ_DRAW_VALUE, reserved) == FALSE)
        return FALSE;
    
    return TRUE;
        
}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayClampResults
//
//  Description:
//      Overlays the results of a clamp on an image
//
//  Parameters:
//      image           -   The image on which the function overlays the results.
//      searchArea      -   The area of the clamp.
//      firstReport     -   The first rake report used in the clamp.
//      secondReport    -   The second rake report used in the clamp (only used
//                          to display results for imaqClampMin).
//      firstClampLine  -   The first perpendicular clamp line.
//      lastClampLine   -   The last perpendicular clamp line.
//      firstEdge       -   The first edge
//      lastEdge        -   The last edge
//      forMax          -   If TRUE, draw results for imaqClampMax function
//                          Else, draw results for imaqClampMin function
//      options         -   Determines which options to draw
//                          This is an optional parameter.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayClampResults( Image* image, const ROI* searchArea, const RakeReport* firstReport, const RakeReport* secondReport, const LineFloat* firstClampLine, const LineFloat* lastClampLine, PointFloat firstEdge, PointFloat lastEdge, 
                             int forMax, const FindEdgeOptions* options ) {

    int showSearchArea;
    int showSearchLines;
    int showEdgesFound;
    int showResult;
	char* overlayName=defaultGroup;
    Point start;
    Point end;
    int i, j;
    
    // Verify that the rake report and line pointers are not NULL
    if( firstReport == NULL || firstClampLine == NULL || lastClampLine == NULL) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // If this is for minimum clamp, verify the second rake report
    if(!forMax) {
        if( secondReport == NULL ) {
            imaqSetError(ERR_NULL_POINTER, NULL);
            return FALSE;
        }
    }

    // Use the supplied display options (if available) or use default options
    if( options ) {
        showSearchArea  = options->showSearchArea;
        showSearchLines = options->showSearchLines;
        showEdgesFound  = options->showEdgesFound;
        showResult      = options->showResult;
		overlayName		= options->groupName;
    }
    else {
        showSearchArea  = DEFAULT_SHOW_SEARCH_AREA;
        showSearchLines = DEFAULT_SHOW_SEARCH_LINES;
        showEdgesFound  = DEFAULT_SHOW_EDGES_FOUND;
        showResult      = DEFAULT_SHOW_RESULTS;
    }
	imaqClearOverlay(image, overlayName);

    // If requested, overlay the search area
    if(showSearchArea) {
		if (imaqOverlayROI(image, searchArea, IMAQ_POINT_AS_CROSS, NULL, overlayName) == FALSE)
            return FALSE;
    }

    // If requested, overlay the search lines
    if(showSearchLines) {
        for ( i = 0; i < firstReport->numRakeLines; i++ ) {
			if (imaqOverlayLine(image, imaqMakePointFromPointFloat(firstReport->rakeLines[i].start), imaqMakePointFromPointFloat(firstReport->rakeLines[i].end), &IMAQ_RGB_BLUE, overlayName) == FALSE)
                return FALSE;
        }
        if(!forMax) {
            for ( i = 0; i < secondReport->numRakeLines; i++ ) {
				if (imaqOverlayLine(image, imaqMakePointFromPointFloat(secondReport->rakeLines[i].start), imaqMakePointFromPointFloat(secondReport->rakeLines[i].end), &IMAQ_RGB_BLUE, overlayName) == FALSE)
                    return FALSE;
            }
        }
    }

    // If requested, overlay the found edges
    if(showEdgesFound) {
        for ( i = 0; i < firstReport->numLinesWithEdges; i++ ) {
            for ( j = 0; j < firstReport->allEdges[i].numEdges; j++ ) {
				if (imaqOverlayRect(image, imaqMakeRect(ROUND_TO_INT(firstReport->allEdges[i].edges[j].y), ROUND_TO_INT(firstReport->allEdges[i].edges[j].x), OVERLAY_FEATURE_SIZE, OVERLAY_FEATURE_SIZE), &IMAQ_RGB_YELLOW, IMAQ_DRAW_VALUE, overlayName) == FALSE)
                    return FALSE;
            }
        }
        if(!forMax) {
            for ( i = 0; i < secondReport->numLinesWithEdges; i++ ) {
                for ( j = 0; j < secondReport->allEdges[i].numEdges; j++ ) {
					if (imaqOverlayRect(image, imaqMakeRect(ROUND_TO_INT(secondReport->allEdges[i].edges[j].y), ROUND_TO_INT(secondReport->allEdges[i].edges[j].x), OVERLAY_FEATURE_SIZE, OVERLAY_FEATURE_SIZE), &IMAQ_RGB_YELLOW, IMAQ_DRAW_VALUE, overlayName) == FALSE)
                        return FALSE;
                }
            }
        }
    }

    // If requested, and the edges are valid, overlay the resutls
    if(showResult && !(firstEdge.x == 0 && firstEdge.y == 0) && !(lastEdge.x == 0 && lastEdge.y == 0 )) {
        // Overlay the result edges
		if (imaqOverlayOval(image, imaqMakeRect(M_OFFSET(firstEdge.y), M_OFFSET(firstEdge.x), OVERLAY_RESULT_SIZE, OVERLAY_RESULT_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
            return FALSE;
		if (imaqOverlayOval(image, imaqMakeRect(M_OFFSET(lastEdge.y), M_OFFSET(lastEdge.x), OVERLAY_RESULT_SIZE, OVERLAY_RESULT_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
            return FALSE;

        // Overlay the two lines perpendicular to the search lines 
        // that correspond to the clamp position
		if (imaqOverlayLine(image, imaqMakePointFromPointFloat(firstClampLine->start), imaqMakePointFromPointFloat(firstClampLine->end), &IMAQ_RGB_RED, overlayName) == FALSE)
            return FALSE;
		if (imaqOverlayLine(image, imaqMakePointFromPointFloat(lastClampLine->start), imaqMakePointFromPointFloat(lastClampLine->end), &IMAQ_RGB_RED, overlayName) == FALSE)
            return FALSE;

        if( forMax ) {
            // Overlay two lines that point inward from the search area to 
            // the clamp position
            start.x = (int)((firstReport->rakeLines[0].start.x + firstReport->rakeLines[firstReport->numRakeLines - 1].start.x) / 2);
            start.y = (int)((firstReport->rakeLines[0].start.y + firstReport->rakeLines[firstReport->numRakeLines - 1].start.y) / 2);
            end.x = (int)((firstClampLine->start.x + firstClampLine->end.x) / 2);
            end.y = (int)((firstClampLine->start.y + firstClampLine->end.y) / 2);
			if (imaqOverlayLineWithArrow(image, start, end, &IMAQ_RGB_RED, FALSE, TRUE, overlayName) == FALSE)
                return FALSE;
            start.x = (int)((firstReport->rakeLines[0].end.x + firstReport->rakeLines[firstReport->numRakeLines - 1].end.x) / 2);
            start.y = (int)((firstReport->rakeLines[0].end.y + firstReport->rakeLines[firstReport->numRakeLines - 1].end.y) / 2);
            end.x = (int)((lastClampLine->start.x + lastClampLine->end.x) / 2);
            end.y = (int)((lastClampLine->start.y + lastClampLine->end.y) / 2);
			if (imaqOverlayLineWithArrow(image, start, end, &IMAQ_RGB_RED, FALSE, TRUE, overlayName) == FALSE)
                return FALSE;
        }
        else {
            // Overlay a single line that points outward from the search area
            // to the clamp position
            start.x = (int)((firstClampLine->start.x + firstClampLine->end.x) / 2);
            start.y = (int)((firstClampLine->start.y + firstClampLine->end.y) / 2);
            end.x = (int)((lastClampLine->start.x + lastClampLine->end.x) / 2);
            end.y = (int)((lastClampLine->start.y + lastClampLine->end.y) / 2);
			if (imaqOverlayLineWithArrow(image, start, end, &IMAQ_RGB_RED, TRUE, TRUE, overlayName) == FALSE)
                return FALSE;
        }
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayCountObjectsResults
//
//  Description:
//      Overlays the results of finding objects on an image
//
//  Parameters:
//      image       -   The image on which the function overlays the results.
//      roi         -   The area of object detection.
//      reports     -   The results of the object detection.
//      reportCount -   The number of reports.
//      options     -   Determines what information to draw.
//                      This is an optional parameter.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayCountObjectsResults( Image* image, const ROI* roi, const ObjectReport* reports, int reportCount, const CountObjectsOptions* options ) {

    int showSearchArea;
    int showObjectCenter;
    int showBoundingBox;
    int i;
    char intString[34];
	char* overlayName = defaultGroup;

    // Use the supplied display options (if available) or use default options
    if( options ) {
        showSearchArea   = options->showSearchArea;
        showObjectCenter = options->showObjectCenter;
        showBoundingBox  = options->showBoundingBox;
		overlayName = options->groupName;
    }
    else {
        showSearchArea   = DEFAULT_SHOW_SEARCH_AREA;
        showObjectCenter = DEFAULT_SHOW_RESULTS;
        showBoundingBox  = DEFAULT_SHOW_RESULTS;
    }

    // If requested, overlay the search area
    if(showSearchArea) {
		if (imaqOverlayROI(image, roi, IMAQ_POINT_AS_CROSS, NULL, overlayName) == FALSE)
            return FALSE;
    }

    // If requested, overlay the object centers
    if(showObjectCenter) {
        for ( i = 0; i < reportCount; i++ ) {
			if (imaqOverlayOval(image, imaqMakeRect(S_OFFSET(reports[i].center.y), S_OFFSET(reports[i].center.x), OVERLAY_FEATURE_SIZE, OVERLAY_FEATURE_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
                return FALSE;
        }
    }

    // If requested, overlay the bounding boxes
    if(showBoundingBox) {
        for ( i = 0; i < reportCount; i++ ) {
			if (imaqOverlayRect(image, reports[i].boundingRect, &IMAQ_RGB_RED, IMAQ_DRAW_VALUE, overlayName) == FALSE)
                return FALSE;
            sprintf (intString, "%d", i);
			if (imaqOverlayText(image, imaqMakePoint(reports[i].boundingRect.left + 1, reports[i].boundingRect.top + 1), intString, &IMAQ_RGB_RED, NULL, overlayName) == FALSE)
                return FALSE;
        }
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayFindCircularEdgeResults
//
//  Description:
//      Overlays the results of finding a circular edge on an image
//
//  Parameters:
//      image       -   The image on which the function overlays the results.
//      roi         -   The area of edge detection.
//      spokeReport -   The results of the spoke used in the edge detection.
//      edgeReport  -   An edge report for the detected edge.
//      options     -   Determines which options to draw.
//                      This is an optional parameter.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int  imaqOverlayFindCircularEdgeResults( Image* image, const ROI* roi, const SpokeReport* spokeReport, const CircularEdgeReport* edgeReport, const FindEdgeOptions* options ) {

    int showSearchArea;
    int showSearchLines;
    int showEdgesFound;
    int showResult;
    int i;
	char* overlayName = defaultGroup;

    // Verify that the rake report and edge report pointers are not NULL
    if( spokeReport == NULL || edgeReport == NULL ) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Use the supplied display options (if available) or use default options
    if( options ) {
        showSearchArea  = options->showSearchArea;
        showSearchLines = options->showSearchLines;
        showEdgesFound  = options->showEdgesFound;
        showResult      = options->showResult;
		overlayName = options->groupName;
    }
    else {
        showSearchArea  = DEFAULT_SHOW_SEARCH_AREA;
        showSearchLines = DEFAULT_SHOW_SEARCH_LINES;
        showEdgesFound  = DEFAULT_SHOW_EDGES_FOUND;
        showResult      = DEFAULT_SHOW_RESULTS;
    }

    // If requested, overlay the search area
    if(showSearchArea) {
		if (imaqOverlayROI(image, roi, IMAQ_POINT_AS_CROSS, NULL, overlayName) == FALSE)
            return FALSE;
    }

    // If requested, overlay the search lines
    if(showSearchLines) {
        for ( i = 0; i < spokeReport->numSpokeLines; i++ ) {
            if( i % 3 == 0 ) {
				if (imaqOverlayLineWithArrow(image, imaqMakePointFromPointFloat(spokeReport->spokeLines[i].start), imaqMakePointFromPointFloat(spokeReport->spokeLines[i].end), &IMAQ_RGB_BLUE, FALSE, TRUE, overlayName) == FALSE)
                    return FALSE;
            }
            else {
				if (imaqOverlayLine(image, imaqMakePointFromPointFloat(spokeReport->spokeLines[i].start), imaqMakePointFromPointFloat(spokeReport->spokeLines[i].end), &IMAQ_RGB_BLUE, overlayName) == FALSE)
                    return FALSE;
            }
        }
    }

    // If requested, overlay the found edges
    if(showEdgesFound) {
        for ( i = 0; i < edgeReport->numCoordinates; i++ ) {
			if (imaqOverlayRect(image, imaqMakeRect(S_OFFSET(edgeReport->coordinates[i].y), S_OFFSET(edgeReport->coordinates[i].x), OVERLAY_FEATURE_SIZE, OVERLAY_FEATURE_SIZE), &IMAQ_RGB_YELLOW, IMAQ_DRAW_VALUE, overlayName) == FALSE)
                return FALSE;
        }
    }

    if(showResult) {
        // Overlay the end points of the straight edge
		if (imaqOverlayOval(image, imaqMakeRect(M_OFFSET(edgeReport->center.y), M_OFFSET(edgeReport->center.x), OVERLAY_RESULT_SIZE, OVERLAY_RESULT_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
            return FALSE;
		if (imaqOverlayOval(image, imaqMakeRect(ROUND_TO_INT(edgeReport->center.y - edgeReport->radius), ROUND_TO_INT(edgeReport->center.x - edgeReport->radius), (int)(edgeReport->radius * 2), (int)(edgeReport->radius * 2)), &IMAQ_RGB_RED, IMAQ_DRAW_VALUE, overlayName) == FALSE)
            return FALSE;
    }

    return TRUE;
    
}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayFindConcentricEdgeResults
//
//  Description:
//      Overlays the results of finding a concentric straight edge on an image
//
//  Parameters:
//      image       -   The image on which the function overlays the results.
//      roi         -   The area of edge detection.
//      rakeReport  -   The results of the rake used in the edge detection.
//      edgeReport  -   An edge report for the detected edge.
//      direction   -   The direction used in the edge detection
//      options     -   Determines which options to draw.
//                      This is an optional parameter.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int  imaqOverlayFindConcentricEdgeResults( Image* image, const ROI* roi, const ConcentricRakeReport* rakeReport, const StraightEdgeReport* edgeReport, ConcentricRakeDirection direction, const FindEdgeOptions* options) {

    int showSearchArea;
    int showSearchLines;
    int showEdgesFound;
    int showResult;
    int i;
    ContourInfo2 *contourInfo;
    ContourID contourID;
	char* overlayName = defaultGroup;

    // Verify that the rake report and edge report pointers are not NULL
    if( rakeReport == NULL || edgeReport == NULL ) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Use the supplied display options (if available) or use default options
    if( options ) {
        showSearchArea  = options->showSearchArea;
        showSearchLines = options->showSearchLines;
        showEdgesFound  = options->showEdgesFound;
        showResult      = options->showResult;
		overlayName		= options->groupName;
    }
    else {
        showSearchArea  = DEFAULT_SHOW_SEARCH_AREA;
        showSearchLines = DEFAULT_SHOW_SEARCH_LINES;
        showEdgesFound  = DEFAULT_SHOW_EDGES_FOUND;
        showResult      = DEFAULT_SHOW_RESULTS;
    }

    // If requested, overlay the search lines
    if(showSearchLines) {
        for ( i = 0; i < rakeReport->numArcs; i++ ) {
			if (imaqOverlayArc(image, &(rakeReport->rakeArcs[i]), &IMAQ_RGB_BLUE, IMAQ_DRAW_VALUE, overlayName) == FALSE)
                return FALSE;
        }
        // Find the inner and outer radii of the annulus.
        if ((contourID = imaqGetContour(roi, 0)) == 0)
            return FALSE;
        if ((contourInfo = imaqGetContourInfo2(roi, contourID)) == NULL)
            return FALSE;
        if (contourInfo->type != IMAQ_ANNULUS) {
            imaqDispose(contourInfo);
            return FALSE;
        }
        // Find the innermost arc that isn't on the annulus and draw an arrow on it.
        if ((rakeReport->rakeArcs[0].boundingBox.height / 2 > (contourInfo->structure.annulus->innerRadius + 1) || rakeReport->numArcs < 2)) {
			if (imaqOverlayArcWithArrow(image, &(rakeReport->rakeArcs[0]), &IMAQ_RGB_BLUE, direction, overlayName) == FALSE) {
                imaqDispose(contourInfo);
                return FALSE;
            }
        } else {
			if (imaqOverlayArcWithArrow(image, &(rakeReport->rakeArcs[1]), &IMAQ_RGB_BLUE, direction, overlayName) == FALSE) {
                imaqDispose(contourInfo);
                return FALSE;
            }
        }
        // Find the outermost arc that isn't on the annulus and draw an arrow on it.
        if ((rakeReport->rakeArcs[rakeReport->numArcs - 1].boundingBox.height / 2 < (contourInfo->structure.annulus->outerRadius - 1)) || rakeReport->numArcs < 2) {
			if (imaqOverlayArcWithArrow(image, &(rakeReport->rakeArcs[rakeReport->numArcs - 1]), &IMAQ_RGB_BLUE, direction, overlayName) == FALSE) {
                imaqDispose(contourInfo);
                return FALSE;
            }
        } else {
			if (imaqOverlayArcWithArrow(image, &(rakeReport->rakeArcs[rakeReport->numArcs - 2]), &IMAQ_RGB_BLUE, direction, overlayName) == FALSE) {
                imaqDispose(contourInfo);
                return FALSE;
            }
        }
        imaqDispose(contourInfo);
        
    }

    // If requested, overlay the search area
    if(showSearchArea) {
		if (imaqOverlayROI(image, roi, IMAQ_POINT_AS_CROSS, NULL, overlayName) == FALSE)
            return FALSE;
    }

    // If requested, overlay the found edges
    if(showEdgesFound) {
        for ( i = 0; i < edgeReport->numCoordinates; i++ ) {
			if (imaqOverlayRect(image, imaqMakeRect(S_OFFSET(edgeReport->coordinates[i].y), S_OFFSET(edgeReport->coordinates[i].x), OVERLAY_FEATURE_SIZE, OVERLAY_FEATURE_SIZE), &IMAQ_RGB_YELLOW, IMAQ_DRAW_VALUE, overlayName) == FALSE)
                return FALSE;
        }
    }

    if(showResult) {
        // Overlay the end points of the straight edge
		if (imaqOverlayOval(image, imaqMakeRect(M_OFFSET(edgeReport->start.y), M_OFFSET(edgeReport->start.x), OVERLAY_RESULT_SIZE, OVERLAY_RESULT_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
            return FALSE;
		if (imaqOverlayOval(image, imaqMakeRect(M_OFFSET(edgeReport->end.y), M_OFFSET(edgeReport->end.x), OVERLAY_RESULT_SIZE, OVERLAY_RESULT_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
            return FALSE;

        // Overlay the straight edge
		if (imaqOverlayLine(image, imaqMakePointFromPointFloat(edgeReport->start), imaqMakePointFromPointFloat(edgeReport->end), &IMAQ_RGB_RED, overlayName) == FALSE)
            return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayFindEdgeResults
//
//  Description:
//      Overlays the results of finding a straight edge on an image
//
//  Parameters:
//      image       -   The image on which the function overlays the results.
//      roi         -   The area of edge detection.
//      rakeReport  -   The results of the rake used in the edge detection.
//      edgeReport  -   An edge report for the detected edge.
//      options     -   Determines which options to draw.
//                      This is an optional parameter.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayFindEdgeResults( Image* image, const ROI* roi, const RakeReport* rakeReport, const StraightEdgeReport* edgeReport, const FindEdgeOptions* options ) {

    int showSearchArea;
    int showSearchLines;
    int showEdgesFound;
    int showResult;
    int i;
	char* overlayName = defaultGroup;

    // Verify that the rake report and edge report pointers are not NULL
    if( rakeReport == NULL || edgeReport == NULL ) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Use the supplied display options (if available) or use default options
    if( options ) {
        showSearchArea  = options->showSearchArea;
        showSearchLines = options->showSearchLines;
        showEdgesFound  = options->showEdgesFound;
        showResult      = options->showResult;
		overlayName		= options->groupName;
    }
    else {
        showSearchArea  = DEFAULT_SHOW_SEARCH_AREA;
        showSearchLines = DEFAULT_SHOW_SEARCH_LINES;
        showEdgesFound  = DEFAULT_SHOW_EDGES_FOUND;
        showResult      = DEFAULT_SHOW_RESULTS;
    }

    // If requested, overlay the search area
    if(showSearchArea) {
		if (imaqOverlayROI(image, roi, IMAQ_POINT_AS_CROSS, NULL, overlayName) == FALSE)
            return FALSE;
    }

    // If requested, overlay the search lines
    if(showSearchLines) {
        for ( i = 0; i < rakeReport->numRakeLines; i++ ) {
            if( i % 3 == 0 ) {
				if (imaqOverlayLineWithArrow(image, imaqMakePointFromPointFloat(rakeReport->rakeLines[i].start), imaqMakePointFromPointFloat(rakeReport->rakeLines[i].end), &IMAQ_RGB_BLUE, FALSE, TRUE, overlayName) == FALSE)
                    return FALSE;
            }
            else {
				if (imaqOverlayLine(image, imaqMakePointFromPointFloat(rakeReport->rakeLines[i].start), imaqMakePointFromPointFloat(rakeReport->rakeLines[i].end), &IMAQ_RGB_BLUE, overlayName) == FALSE)
                    return FALSE;
            }
        }
    }

    // If requested, overlay the found edges
    if(showEdgesFound) {
        for ( i = 0; i < edgeReport->numCoordinates; i++ ) {
			if (imaqOverlayRect(image, imaqMakeRect(S_OFFSET(edgeReport->coordinates[i].y), S_OFFSET(edgeReport->coordinates[i].x), OVERLAY_FEATURE_SIZE, OVERLAY_FEATURE_SIZE), &IMAQ_RGB_YELLOW, IMAQ_DRAW_VALUE, overlayName) == FALSE)
                return FALSE;
        }
    }

    if(showResult) {
        // Overlay the end points of the straight edge
		if (imaqOverlayOval(image, imaqMakeRect(M_OFFSET(edgeReport->start.y), M_OFFSET(edgeReport->start.x), OVERLAY_RESULT_SIZE, OVERLAY_RESULT_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
            return FALSE;
		if (imaqOverlayOval(image, imaqMakeRect(M_OFFSET(edgeReport->end.y), M_OFFSET(edgeReport->end.x), OVERLAY_RESULT_SIZE, OVERLAY_RESULT_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
            return FALSE;

        // Overlay the straight edge
		if (imaqOverlayLine(image, imaqMakePointFromPointFloat(edgeReport->start), imaqMakePointFromPointFloat(edgeReport->end), &IMAQ_RGB_RED, overlayName) == FALSE)
            return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayFindPatternResults
//
//  Description:
//      Overlays the results of matching patterns in an image
//
//  Parameters:
//      image       -   The image on which the function overlays the results.
//      roi         -   The area of pattern matching.
//      matches     -   The pattern matches to overlay.
//      numMatches  -   The number of pattern matches
//      options     -   Determines which options to draw.
//                      This is an optional parameter.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayFindPatternResults( Image* image, const ROI* roi, const PatternMatch* matches, int numMatches, const FindPatternOptions* options ) {

    int showSearchArea;
    int showResult;
    int i;
	char* overlayName = defaultGroup;

    // Use the supplied display options (if available) or use default options
    if( options ) {
        showSearchArea  = options->showSearchArea;
        showResult      = options->showResult;
		overlayName		= options->groupName;
    }
    else {
        showSearchArea  = DEFAULT_SHOW_SEARCH_AREA;
        showResult      = DEFAULT_SHOW_RESULTS;
    }

    // If requested, overlay the search area
    if(showSearchArea) {
		if (imaqOverlayROI(image, roi, IMAQ_POINT_AS_CROSS, NULL, overlayName) == FALSE)
            return FALSE;
    }

    // If requested, overlay each of the matches
    if(showResult) {
        for( i = 0; i < numMatches; i++ ) {
			if (imaqOverlayPatternMatch(image, &(matches[i]), &IMAQ_RGB_RED, overlayName) == FALSE)
                return FALSE;
        }
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayFindTransformPattern
//
//  Description:
//      Overlays the results of finding a transform using a pattern
//
//  Parameters:
//      image       -   The image on which the function overlays the results.
//      searchRect  -   The area of pattern search detection.
//      match       -   The resulting pattern match
//      options     -   Determines which options to draw.
//                      This is an optional parameter.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayFindTransformPattern (Image* image, RotatedRect searchRect, const PatternMatch* match, const FindTransformPatternOptions* options ) {

    int showSearchArea;
    int showFeatureFound;
    int showResult;
	char* overlayName = defaultGroup;

    // Verify that the pattern match pointer is not NULL
    if( match == NULL ) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Use the options (if provided) to determine what to show
    if( options ) {
        showSearchArea   = options->showSearchArea;
        showFeatureFound = options->showFeatureFound;
        showResult       = options->showResult;
		overlayName		 = options->groupName;
    }
    else {
        showSearchArea   = DEFAULT_SHOW_SEARCH_AREA;
        showFeatureFound = DEFAULT_SHOW_FEATURE_FOUND;
        showResult       = DEFAULT_SHOW_RESULTS;
    }

    // Overlay the search area
    if(showSearchArea) {
		if (imaqOverlayRect(image, imaqMakeRectFromRotatedRect(searchRect), &IMAQ_RGB_GREEN, IMAQ_DRAW_VALUE, overlayName) == FALSE)
            return FALSE;
    }

    // Overlay the pattern match
    if(showFeatureFound) {
        if( imaqOverlayPatternMatch( image, match, &IMAQ_RGB_YELLOW, overlayName ) == FALSE )
            return FALSE;
    }

    // Overlay the new axis
    if(showResult) {
		if (imaqOverlayOval(image, imaqMakeRect(M_OFFSET(match->corner[3].y), M_OFFSET(match->corner[3].x), OVERLAY_RESULT_SIZE, OVERLAY_RESULT_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
            return FALSE;
		if (imaqOverlayLineWithArrow(image, imaqMakePointFromPointFloat(match->corner[3]), imaqMakePointFromPointFloat(match->corner[2]), &IMAQ_RGB_RED, FALSE, TRUE, overlayName) == FALSE)
            return FALSE;
		if (imaqOverlayLineWithArrow(image, imaqMakePointFromPointFloat(match->corner[3]), imaqMakePointFromPointFloat(match->corner[0]), &IMAQ_RGB_RED, FALSE, TRUE, overlayName) == FALSE)
            return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayFindTransformRects
//
//  Description:
//      Overlays the results of finding a transform using one or two rectangles
//
//  Parameters:
//      image               -   The image on which the function overlays the 
//                              results.
//      mainROI             -   The area of first seach rectangle.
//      mainRakeReport      -   The results of the first search rake.
//      secondaryRakeReport -   The results of the second search rake.
//      origin              -   The origin of the coordinate system.
//      mainAxisEnd         -   The end of the main axis of the coordinate 
//                              system.
//      secondaryAxisEnd    -   The end of the secondary axis of the coordinate 
//                              system.
//      optionsRect         -   Determines which options to draw in the single
//                              rectangle case.
//                              This is an optional parameter.  
//      optionsRects        -   Determines which options to draw in the two
//                              rectangles case.
//                              This is an optional parameter.  
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayFindTransformRects( Image* image, const ROI* mainROI, const RakeReport* mainRakeReport, const RakeReport* secondaryRakeReport, PointFloat origin, PointFloat mainAxisEnd, 
                                   PointFloat secondaryAxisEnd, const FindTransformRectOptions* optionsRect, const FindTransformRectsOptions* optionsRects ) {

    int showSearchArea;
    int showSearchLines;
    int showEdgesFound;
    int showResult;
    int i;
	char* overlayName = defaultGroup;

    // Verify that the rake report pointers are not NULL
    if( mainRakeReport == NULL || secondaryRakeReport == NULL ) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Use the supplied display options (if available) or use default options
    if( optionsRect ) {
        showSearchArea  = optionsRect->showSearchArea;
        showSearchLines = optionsRect->showSearchLines;
        showEdgesFound  = optionsRect->showEdgesFound;
        showResult      = optionsRect->showResult;
		overlayName		= optionsRect->groupName;
    }
    else if( optionsRects ) {
        showSearchArea  = optionsRects->showSearchArea;
        showSearchLines = optionsRects->showSearchLines;
        showEdgesFound  = optionsRects->showEdgesFound;
        showResult      = optionsRects->showResult;
		overlayName		= optionsRects->groupName;
    }
    else {
        showSearchArea  = DEFAULT_SHOW_SEARCH_AREA;
        showSearchLines = DEFAULT_SHOW_SEARCH_LINES;
        showEdgesFound  = DEFAULT_SHOW_EDGES_FOUND;
        showResult      = DEFAULT_SHOW_RESULTS;
    }

    // If requested, overlay the search area
    if(showSearchArea) {
		if (imaqOverlayROI(image, mainROI, IMAQ_POINT_AS_CROSS, NULL, overlayName) == FALSE)
            return FALSE;
    }

    // If requested, overlay the search lines
    if(showSearchLines) {
        for ( i = 0; i < mainRakeReport->numRakeLines; i++ ) {
			if (imaqOverlayLine(image, imaqMakePointFromPointFloat(mainRakeReport->rakeLines[i].start), imaqMakePointFromPointFloat(mainRakeReport->rakeLines[i].end), &IMAQ_RGB_BLUE, overlayName) == FALSE)
                return FALSE;
        }
        for ( i = 0; i < secondaryRakeReport->numRakeLines; i++ ) {
			if (imaqOverlayLine(image, imaqMakePointFromPointFloat(secondaryRakeReport->rakeLines[i].start), imaqMakePointFromPointFloat(secondaryRakeReport->rakeLines[i].end), &IMAQ_RGB_BLUE, overlayName) == FALSE)
                return FALSE;
        }
    }

    // If requested, overlay the found edges
    if(showEdgesFound) {
        for ( i = 0; i < mainRakeReport->numFirstEdges; i++ ) {
			if (imaqOverlayRect(image, imaqMakeRect(S_OFFSET(mainRakeReport->firstEdges[i].y), S_OFFSET(mainRakeReport->firstEdges[i].x), OVERLAY_FEATURE_SIZE, OVERLAY_FEATURE_SIZE), &IMAQ_RGB_YELLOW, IMAQ_DRAW_VALUE, overlayName) == FALSE)
                return FALSE;
        }
        for ( i = 0; i < secondaryRakeReport->numFirstEdges; i++ ) {
			if (imaqOverlayRect(image, imaqMakeRect(S_OFFSET(secondaryRakeReport->firstEdges[i].y), S_OFFSET(secondaryRakeReport->firstEdges[i].x), OVERLAY_FEATURE_SIZE, OVERLAY_FEATURE_SIZE), &IMAQ_RGB_YELLOW, IMAQ_DRAW_VALUE, overlayName) == FALSE)
                return FALSE;
        }
    }

    if(showResult) {
		if (imaqOverlayOval(image, imaqMakeRect(M_OFFSET(origin.y), M_OFFSET(origin.x), OVERLAY_RESULT_SIZE, OVERLAY_RESULT_SIZE), &IMAQ_RGB_RED, IMAQ_PAINT_VALUE, overlayName) == FALSE)
            return FALSE;
		if (imaqOverlayLineWithArrow(image, imaqMakePointFromPointFloat(origin), imaqMakePointFromPointFloat(mainAxisEnd), &IMAQ_RGB_RED, FALSE, TRUE, overlayName) == FALSE)
            return FALSE;
		if (imaqOverlayLineWithArrow(image, imaqMakePointFromPointFloat(origin), imaqMakePointFromPointFloat(secondaryAxisEnd), &IMAQ_RGB_RED, FALSE, TRUE, overlayName) == FALSE)
            return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayLineWithArrow
//
//  Description:
//      Overlays a line with arrows on one or both ends
//
//  Parameters:
//      image       -   The image on which the function overlays the line.
//      start       -   The beginning of the line.
//      end         -   The end of the line.
//      color       -   The color of the line.
//      startArrow  -   If TRUE, the function overlays an arrow at the start
//                      of the line.
//      endArrow    -   If TRUE, the function overlays an arrow at the end
//                      of the line.
//      reserved    -   Reserved. Set to NULL.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayLineWithArrow (Image* image, Point start, Point end, const RGBValue* color, int startArrow, int endArrow, void* reserved) {
    
    // First, overlay the line
    if ( imaqOverlayLine(image, start, end, color, reserved ) == FALSE )
        return FALSE;
    
    // Then, overlay the requested arrows
    if( startArrow ) 
        if ( imaqOverlayArrow( image, start, end, color, reserved) == FALSE )
            return FALSE;
        
    if( endArrow )
        if ( imaqOverlayArrow( image, end, start, color, reserved) == FALSE )
            return FALSE;
            
    return TRUE;            

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayArcWithArrow
//
//  Description:
//      Overlays an arc with an arrow in the middle.
//
//  Parameters:
//      image       -   The image on which the function overlays the line.
//      arc         -   The arc to overlay.
//      color       -   The color of the arc.
//      direction   -   The direction to draw the arrow in.
//      reserved    -   Reserved. Set to NULL.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayArcWithArrow (Image* image, ArcInfo* arc, const RGBValue* color, ConcentricRakeDirection direction, void* reserved) {
    PointFloat center;
    PointFloat middle;
    PointFloat guidePoint;
    Point middleToUse;
    Point guidePointToUse;
    double middleAngle;
    double newAngle;
    int directionFactor;
    // First, overlay the arc
    if ( imaqOverlayArc(image, arc, color, IMAQ_DRAW_VALUE, reserved ) == FALSE )
        return FALSE;
        
    
    center.x = (float)(arc->boundingBox.left + arc->boundingBox.width / 2.0);
    center.y = (float)(arc->boundingBox.top + arc->boundingBox.height / 2.0);
    
    if (arc->boundingBox.width == 0 || arc->boundingBox.height == 0)
        return FALSE;
    if (direction == IMAQ_CLOCKWISE)
        directionFactor = 1;
    else
        directionFactor = -1;

    while (arc->startAngle >= 360)
        arc->startAngle -= 360;
    while (arc->startAngle < 0)
        arc->startAngle += 360;
    while (arc->endAngle >= 360)
        arc->endAngle -= 360;
    while (arc->endAngle < 0)
        arc->endAngle += 360;
        
    middleAngle = (arc->startAngle + arc->endAngle) / 2.0;
    if (middleAngle <= arc->startAngle || middleAngle >= arc->endAngle) {
        middleAngle += 180;
        if (middleAngle >= 360)
            middleAngle -= 360;
    }
    middle.x = (float)(center.x + cos(middleAngle * IMAQ_PI / 180) * arc->boundingBox.width / 2.0);
    middle.y = (float)(center.y - sin(middleAngle * IMAQ_PI / 180) * arc->boundingBox.height / 2.0);
    // Now, calculate the guide point, and overlay the arrow.
    newAngle = middleAngle + directionFactor * 10;
    guidePoint.x = (float)(center.x + cos(newAngle * IMAQ_PI / 180) * arc->boundingBox.width / 2.0);
    guidePoint.y = (float)(center.y - sin(newAngle * IMAQ_PI / 180) * arc->boundingBox.height / 2.0);
    middleToUse.x = ROUND_TO_INT(middle.x);
    middleToUse.y = ROUND_TO_INT(middle.y);
    guidePointToUse.x = ROUND_TO_INT(guidePoint.x);
    guidePointToUse.y = ROUND_TO_INT(guidePoint.y);
    if (imaqOverlayArrow( image, middleToUse, guidePointToUse, color, reserved) == FALSE )
        return FALSE;
    
    return TRUE;            

}

//////////////////////////////////////////////////////////////////////////////
//
//  imaqOverlayPatternMatch
//
//  Description:
//      Overlays the location of a pattern match
//
//  Parameters:
//      image       -   The image on which the function overlays the line.
//      match       -   Information about the pattern match.
//      color       -   The color of the pattern match overlay.
//      reserved    -   Reserved. Set to NULL.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqOverlayPatternMatch( Image* image, const PatternMatch* match, const RGBValue* color, void* reserved ) {

    Point boundingRect[4];  
    int i;

    // Overlay the center of the pattern match
    if( imaqOverlayOval( image, imaqMakeRect( L_OFFSET(match->position.y), L_OFFSET(match->position.x), OVERLAY_PATTERN_SIZE, OVERLAY_PATTERN_SIZE), color, IMAQ_DRAW_VALUE, reserved ) == FALSE )
        return FALSE;

    // Overlay a crosshair at the center of the pattern match
    if( imaqOverlayLine( image, imaqMakePoint(ROUND_TO_INT(match->position.x - OVERLAY_PATTERN_SIZE),ROUND_TO_INT(match->position.y)),imaqMakePoint(ROUND_TO_INT(match->position.x+OVERLAY_PATTERN_SIZE),ROUND_TO_INT(match->position.y)),color,reserved)==FALSE)
        return FALSE;
    if( imaqOverlayLine( image, imaqMakePoint(ROUND_TO_INT(match->position.x),ROUND_TO_INT(match->position.y - OVERLAY_PATTERN_SIZE)),imaqMakePoint(ROUND_TO_INT(match->position.x),ROUND_TO_INT(match->position.y+OVERLAY_PATTERN_SIZE)),color,reserved)==FALSE)
        return FALSE;

    // Overlay the bounding rectangle of the pattern match
    for ( i = 0; i < 4; i++ ) {
        boundingRect[i].x = ROUND_TO_INT(match->corner[i].x);
        boundingRect[i].y = ROUND_TO_INT(match->corner[i].y);
    }
    if ( imaqOverlayClosedContour(image, boundingRect, 4, color, IMAQ_DRAW_VALUE, reserved ) == FALSE )
        return FALSE;

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqPrepareForExit
//
//  Description:
//      Prepares a machine vision function to exit on failure
//
//  Parameters:
//      error           -   The error code.
//      functionName    -   The name of the function that causes the error.
//      ...             -   A list of pointers to things that need to be 
//                          disposed before exiting the function. The last
//                          pointer in the list should always be set to NULL.
//
//  Return Value:
//      None
//
//////////////////////////////////////////////////////////////////////////////
void imaqPrepareForExit( int error, const char* functionName, ... ) {

    va_list disposalPtrList;
    void* disposalPtr; 
    
    // Get the start of the variable list
    va_start( disposalPtrList, functionName );
    disposalPtr = va_arg( disposalPtrList, void* );
    while( disposalPtr != NULL ) {
        // Dispose of structures allocated by IMAQ Vision functions
        imaqDispose(disposalPtr);
        disposalPtr = va_arg( disposalPtrList, void* );
    }
    // Reset variable arguments.
    va_end( disposalPtrList );
    
    // Set the error information
    imaqSetError(error, functionName);

    return;
    
}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqSetAxisOrienation
//
//  Description:
//      Sets the axis orientation based on the direction of each axis
//
//  Parameters:
//      mainAxisDirection       -   The rectangle to split.
//      secondaryAxisDirection  -   The drection of the search; the split occurs in the
//      orientation -   The resulting first rectangle.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqSetAxisOrienation( RakeDirection mainAxisDirection, RakeDirection secondaryAxisDirection, AxisOrientation* orientation ) {

    // Set the orientation based on each direction
    switch (mainAxisDirection) {
        case IMAQ_LEFT_TO_RIGHT:
            switch (secondaryAxisDirection) {
                case IMAQ_TOP_TO_BOTTOM:
                    *orientation = IMAQ_DIRECT;
                    break;
                case IMAQ_BOTTOM_TO_TOP:
                    *orientation = IMAQ_INDIRECT;
                    break;
                default:
                    imaqSetError( ERR_INVALID_RAKE_DIRECTION, NULL );
                    return FALSE;
            }
            break;
        case IMAQ_RIGHT_TO_LEFT:
            switch (secondaryAxisDirection) {
                case IMAQ_TOP_TO_BOTTOM:
                    *orientation = IMAQ_INDIRECT;
                    break;
                case IMAQ_BOTTOM_TO_TOP:
                    *orientation = IMAQ_DIRECT;
                    break;
                default:
                    imaqSetError( ERR_INVALID_RAKE_DIRECTION, NULL );
                    return FALSE;
            }
            break;
        case IMAQ_TOP_TO_BOTTOM:
            switch (secondaryAxisDirection) {
                case IMAQ_LEFT_TO_RIGHT:
                    *orientation = IMAQ_INDIRECT;
                    break;
                case IMAQ_RIGHT_TO_LEFT:
                    *orientation = IMAQ_DIRECT;
                    break;
                default:
                    imaqSetError( ERR_INVALID_RAKE_DIRECTION, NULL );
                    return FALSE;
            }
            break;
        case IMAQ_BOTTOM_TO_TOP:
            switch (secondaryAxisDirection) {
                case IMAQ_LEFT_TO_RIGHT:
                    *orientation = IMAQ_DIRECT;
                    break;
                case IMAQ_RIGHT_TO_LEFT:
                    *orientation = IMAQ_INDIRECT;
                    break;
                default:
                    imaqSetError( ERR_INVALID_RAKE_DIRECTION, NULL );
                    return FALSE;
            }
            break;
        default:
            imaqSetError( ERR_INVALID_RAKE_DIRECTION, NULL );
            return FALSE;
    }

    return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
//
//  imaqSplitRotatedRectangle
//
//  Description:
//      Splits a rotated rectangle into two rotated rectangles
//
//  Parameters:
//      rectToSplit -   The rectangle to split.
//      direction   -   The drection of the search; the split occurs in the
//                      perpendicular direction
//      firstRect   -   The resulting first rectangle.
//      secondRect  -   The resulting second rectangle.
//
//  Return Value:
//      TRUE if successful, FALSE if there was an error
//
//////////////////////////////////////////////////////////////////////////////
int imaqSplitRotatedRectangle( RotatedRect rectToSplit, RakeDirection direction, RotatedRect* firstRect, RotatedRect* secondRect ) {
    
    float xAdjustment, yAdjustment, shiftFactor;

    // Verify that the two return rectangle pointers are not NULL
    if( firstRect == NULL || secondRect == NULL ) {
        imaqSetError(ERR_NULL_POINTER, NULL);
        return FALSE;
    }

    // Transfer the angle to each rectangle (the direction does not effect the 
    // angle).
    firstRect->angle   = rectToSplit.angle;
    secondRect->angle  = rectToSplit.angle;

    // Determine which direction to split the rectangle
    switch ( direction ) {
        case IMAQ_LEFT_TO_RIGHT:
        case IMAQ_RIGHT_TO_LEFT:
            // Split the rectangle down the verticle axis
            firstRect->width   = rectToSplit.width / 2;
            firstRect->height  = rectToSplit.height;
            secondRect->width  = rectToSplit.width / 2;
            secondRect->height = rectToSplit.height;
            // Calculate the shift for the left coordinate of the two rects
            shiftFactor = (float)(rectToSplit.width / 4.0);
            // Set the size of the adjustment
            xAdjustment = (float)(cos(rectToSplit.angle * IMAQ_PI / 180) * shiftFactor);
            yAdjustment = (float)(sin(rectToSplit.angle * IMAQ_PI / 180) * shiftFactor);
            // Fix the directions for the adjustments
            if( direction == IMAQ_LEFT_TO_RIGHT) {
                firstRect->left  = (int)(rectToSplit.left - xAdjustment  + (shiftFactor));
                firstRect->top   = (int)(rectToSplit.top  + yAdjustment);
                secondRect->left = (int)(rectToSplit.left + xAdjustment  + (shiftFactor));
                secondRect->top  = (int)(rectToSplit.top  - yAdjustment);
            }
            else {
                firstRect->left  = (int)(rectToSplit.left + xAdjustment + (shiftFactor));
                firstRect->top   = (int)(rectToSplit.top  - yAdjustment);
                secondRect->left = (int)(rectToSplit.left - xAdjustment + (shiftFactor));
                secondRect->top  = (int)(rectToSplit.top  + yAdjustment);
            }
            break;
        case IMAQ_TOP_TO_BOTTOM:
        case IMAQ_BOTTOM_TO_TOP:
            // Split the rectangle down the horizontal axis
            firstRect->width   = rectToSplit.width;
            firstRect->height  = rectToSplit.height / 2;
            secondRect->width  = rectToSplit.width;
            secondRect->height = rectToSplit.height / 2;
            // Calculate the shift for the top coordinate of the two rects
            shiftFactor = (float)(rectToSplit.height / 4.0);
            // Set the size of the adjustment
            xAdjustment = (float)(sin(rectToSplit.angle * IMAQ_PI / 180) * shiftFactor);
            yAdjustment = (float)(cos(rectToSplit.angle * IMAQ_PI / 180) * shiftFactor);
            // Fix the directions for the adjustments
            if( direction == IMAQ_TOP_TO_BOTTOM) {
                firstRect->left  = (int)(rectToSplit.left - xAdjustment);
                firstRect->top   = (int)(rectToSplit.top  - yAdjustment + (shiftFactor));
                secondRect->left = (int)(rectToSplit.left + xAdjustment);
                secondRect->top  = (int)(rectToSplit.top  + yAdjustment  + (shiftFactor));
            }
            else {
                firstRect->left  = (int)(rectToSplit.left + xAdjustment);
                firstRect->top   = (int)(rectToSplit.top  + yAdjustment + (shiftFactor));
                secondRect->left = (int)(rectToSplit.left - xAdjustment);
                secondRect->top  = (int)(rectToSplit.top  - yAdjustment + (shiftFactor));
            }
            break;
    }

    return TRUE;
    
}
