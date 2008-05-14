//  Author(s): A.j. (Hannes) pretorius
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  LICENSE_1_0.txt)
//
/// \file ./parser.cpp

// --- parser.cpp ---------------------------------------------------
// (c) 2007  -  A.J. Pretorius  -  Eindhoven University of Technology
// ---------------------------  *  ----------------------------------


#include "parser.h"


// -- constructors and destructor -----------------------------------


// --------------------------
Parser::Parser( Mediator* m )
      : Colleague( m )
// --------------------------
{
    delims = "() \"";
}


// --------------
Parser::~Parser()
// --------------
{}


// -- parsing functions ---------------------------------------------


// ------------------------------------------
int Parser::getFileSize( const string &path )
// ------------------------------------------
// ------------------------------------------------------------------
// This function returns the size of the file identified by 'path' in
// bytes. Thanks to http://www.cplusplus.com/doc/tutorial/files.html
// ------------------------------------------------------------------
{
    int result = 0;
    ifstream file;
    
    int begin  = 0;
    int end    = 0;
    
    file.open( path.c_str() );
    if ( !file.is_open() )
    {
        string* msg = new string( 
            "Error opening file for computing file size." );
        throw msg;
    }

    try
    {
        // get starting position
        begin = file.tellg();
        
        // seek and set current position to 'EOF'
        file.seekg( 0, ios::end );
        end = file.tellg();

        // calc size
        result = end - begin;
    }
    catch ( ... )
    {
        string* msg = new string( 
            "Error computing file size." );
        throw msg;
    }
    file.close();
    
    return result;
}


// -----------------------
void Parser::parseFSMFile( 
    const string &path,
    Graph* graph )
// -----------------------
// ------------------------------------------------------------------
// Parse the file identified by 'fileName' by calling:
//  - Parser::parseStateVarDescr()
//  - Parser::parseStates()
//  - Parser::parseTransitions()
// Also, report to 'mediator' on the current progress: number of bytes
// allready read.
// ------------------------------------------------------------------
{
    ifstream file;
    string line = "";
    int sect = 0;
    int lineCnt = 0;
    int byteCnt = 0;
   
    file.open( path.c_str() );
    if ( !file.is_open() )
    {
        string* msg = new string( 
            "Error opening file for parsing." );
        throw msg;
    }

    try
    {
        while ( getline( file, line ) )
        {
            ++lineCnt;
            // add size of line + 1 for EOL char
            byteCnt = byteCnt + line.size() + 1;

            if ( lineCnt % 1000 == 0 )
                mediator->updateProgress( byteCnt );
            
            // linux, mac and windows EOL
            if ( line == "---"   || 
                 line == "---\n" || 
                 line == "---\r" ||
                 line == "---\0" )
                ++sect;
            else
            {
                line += '\n';
                switch ( sect )
                {
                    case 0:
                        parseStateVarDescr( 
                            line, 
                            graph );
                        break;

                    case 1:
                        parseStates( 
                            line, 
                            graph );
                        break;

                    case 2:
                        parseTransitions( 
                            lineCnt,
                            line,
                            graph );
                        break;

                    default:
                        break;
                }
            }
        }

        file.close();
    }
    catch ( const string* msg )
    {
        throw msg;
    }
}


// ---------------------------
void Parser::writeFSMFile(
    const string &path,
    Graph* graph )
// ---------------------------
{
    size_t begIdx, endIdx;
    string fileName;
    string delims    = "\\/";
    
    ofstream file;
    string line = "";
    
    int lineCnt = 0;
    int lineTot = 0;
    
    file.open( path.c_str() );
    if ( !file.is_open() )
    {
        string* msg = new string( 
            "Error opening file for writing." );
        throw msg;
    }

    try
    {
        // get filename
        begIdx = path.find_last_of( delims );
        if ( begIdx == string::npos )
            begIdx = 0;
        else
            begIdx += 1;
        endIdx   = path.size();
        fileName = path.substr( begIdx, endIdx-begIdx );
        
        // init progress
        lineCnt = 0;
        lineTot = graph->getSizeAttributes() + graph->getSizeNodes() + graph->getSizeEdges();
        mediator->initProgress(
            "Saving file",
            "Writing " + fileName,
            lineTot );
    
        
        // write state variable description
        {
        for ( int i = 0; i < graph->getSizeAttributes(); ++i )
        {
            line  = "";
            line.append( graph->getAttribute( i )->getName() );
            line.append( "(" );
            line.append( Utils::intToStr( graph->getAttribute( i )->getSizeOrigValues() ) );
            line.append( ") " );
            line.append( graph->getAttribute( i )->getType() );
            line.append( " " );

            if ( graph->getAttribute(i)->getSizeOrigValues() != 0 )
            {
                for ( int j = 0; j < graph->getAttribute( i )->getSizeCurValues(); ++j )
                {
                    line.append( "\"" );
                    line.append( graph->getAttribute( i )->getCurValue( j )->getValue() );
                    line.append( "\"" );

                    if ( j < graph->getAttribute( i )->getSizeCurValues()-1 )
                        line.append( " " );
                }
            }
            else
            {
                line.append( "[" );
                line.append( Utils::dblToStr( graph->getAttribute( i )->getLowerBound() ) );
                line.append( ", " );
                line.append( Utils::dblToStr( graph->getAttribute( i )->getUpperBound() ) );
                line.append( "]" );
            }

            line.append( "\n" );

            file << line;

            if ( lineCnt % 1000 == 0 )
                mediator->updateProgress( lineCnt );
            ++lineCnt;
        }
        }
        
        // write state vectors
        line = "---\n";
        file << line;

        {
        for ( int i = 0; i < graph->getSizeNodes(); ++i )
        {
            line = "";
            
            for ( int j = 0; j < graph->getNode(i)->getSizeTuple(); ++j )
            {
                line.append( Utils::dblToStr( graph->getNode(i)->getTupleVal(j) ) );
                
                if ( j < graph->getNode( i )->getSizeTuple()-1 )
                    line.append( " " );
            }

            line.append( "\n" );

            file << line;

            if ( lineCnt % 1000 == 0 )
                mediator->updateProgress( lineCnt );
            ++lineCnt;
        }
        }

        // write transitions
        line = "---\n";
        file << line;

        {
        for ( int i = 0; i < graph->getSizeEdges(); ++i )
        {
            line = "";
            
            line.append( Utils::intToStr( graph->getEdge(i)->getInNode()->getIndex()+1 ) );
            line.append( " " );
            line.append( Utils::intToStr( graph->getEdge(i)->getOutNode()->getIndex()+1 ) );
            line.append( " \"" );
            line.append( graph->getEdge(i)->getLabel() );
            line.append( "\"" );

            line.append( "\n" );

            file << line;

            if ( lineCnt % 1000 == 0 )
                mediator->updateProgress( lineCnt );
            ++lineCnt;
        }
        }
        
        file.close();

        // close progress
        mediator->closeProgress();
    }
    catch ( const string* msg )
    {
        throw msg;
    }    
}


// -----------------------------------------------------
void Parser::parseAttrConfig(
    const string &path,
    Graph* graph,
    map< int, int > &attrIdxFrTo,
    map< int, vector< string > > &attrCurDomains,
    map< int, map< int, int  > > &attrOrigToCurDomains )
// -----------------------------------------------------
{
    TiXmlDocument doc( path.c_str() );

    if ( doc.LoadFile() == true )
    {
        TiXmlElement* curNode    = NULL;
        
        try
        {
            
            curNode = doc.FirstChildElement();
            
            if ( curNode != NULL )
            {
                attrIdxFrTo.clear();
                parseAttrConfig(
                    graph,
                    attrIdxFrTo,
                    attrCurDomains,
                    attrOrigToCurDomains,
                    curNode );
            }

            curNode    = NULL;
        }
        catch ( const string* msg )
        {
            curNode = NULL;

            string* errMsg = new string( "Error loading attribute configuration." );
            
            errMsg->append( "\n" );
            errMsg->append( *msg );

            delete msg;
            msg = NULL;
            
            throw errMsg;
        }
    }
    else
    {
        string* msg = new string( "Error opening attribute configuration file." );
        throw msg; 
    }
}


// --------------------------
void Parser::writeAttrConfig(
    const string &path,
    Graph* graph )
// --------------------------
{
    try
	{
        // new xml document
        TiXmlDocument     doc;
        TiXmlDeclaration* decl;
        TiXmlElement*     conf;
        TiXmlElement*     file;
        TiXmlElement*     attr;
        TiXmlElement*     name;
        TiXmlElement*     type;
        TiXmlElement*     card;
        TiXmlElement*     domn;
        TiXmlElement*     valu;
        TiXmlElement*     map;
        TiXmlElement*     pos;
        	    
        // document declaration
        decl = new TiXmlDeclaration( "1.0", "", "" );  
	    doc.LinkEndChild( decl );

        // configuration
	    conf = new TiXmlElement( "Configuration" );  
	    doc.LinkEndChild( conf );
 
        // file name
	    file = new TiXmlElement( "File" );  
	    conf->LinkEndChild( file );
        file->LinkEndChild( new TiXmlText( graph->getFileName().c_str() ) );
        
        // attributes
        for ( int i = 0; i < graph->getSizeAttributes(); ++i )
        {
            attr = new TiXmlElement( "Attribute" );
            conf->LinkEndChild( attr );
                
            // name
            name = new TiXmlElement( "Name" );
            attr->LinkEndChild( name );
            name->LinkEndChild( 
                new TiXmlText( 
                    graph->getAttribute(i)->getName().c_str() ) );

            // type
            type = new TiXmlElement( "Type" );
            attr->LinkEndChild( type );
            type->LinkEndChild( 
                new TiXmlText( 
                    graph->getAttribute(i)->getType().c_str() ) );

            // cardinality
            card = new TiXmlElement( "OriginalCardinality" );
            attr->LinkEndChild( card );
            card->LinkEndChild( 
                new TiXmlText( 
                    Utils::intToStr( 
                        graph->getAttribute(i)->getSizeOrigValues() ).c_str() ) );

            /*
            // original domain
            domn = new TiXmlElement( "OriginalDomain" );
            attr->LinkEndChild( domn );
            {
            for ( int j = 0; j < graph->getAttribute(i)->getSizeOrigValues(); ++j )
            {
                // value
                valu = new TiXmlElement( "Value" );
                domn->LinkEndChild( valu );
                valu->LinkEndChild(
                    new TiXmlText( 
                        graph->getAttribute(i)->getOrigValue(j)->getValue().c_str() ) );
            }
            }
            */

            // current domain
            domn = new TiXmlElement( "CurrentDomain" );
            attr->LinkEndChild( domn );
            {
            for ( int j = 0; j < graph->getAttribute(i)->getSizeCurValues(); ++j )
            {
                // value
                valu = new TiXmlElement( "Value" );
                domn->LinkEndChild( valu );
                valu->LinkEndChild(
                    new TiXmlText( 
                        graph->getAttribute(i)->getCurValue(j)->getValue().c_str() ) );
            }
            }

            // mapping from original to current domain
            // current domain
            map = new TiXmlElement( "OriginalToCurrent" );
            attr->LinkEndChild( map );
            {
            for ( int j = 0; j < graph->getAttribute(i)->getSizeOrigValues(); ++j )
            {
                // value
                pos = new TiXmlElement( "CurrentPosition" );
                map->LinkEndChild( pos );
                pos->LinkEndChild(
                    new TiXmlText( 
                        Utils::intToStr(
                            graph->getAttribute(i)->mapToValue(j)->getIndex() ).c_str() ) );
            }
            }
        }
        
        doc.SaveFile( path.c_str() );
        
        decl = NULL;
        conf = NULL;
        file = NULL;
        attr = NULL;
        name = NULL;
        type = NULL;
        card = NULL;
        domn = NULL;
        valu = NULL;
        map  = NULL;
        pos  = NULL;
    }
	catch ( ... )
	{
		string* msg = new string(
            "Error saving attribute configuration." );
        throw msg;
	}
}


// -----------------------
void Parser::parseDiagram(
    const string &path,
    Graph* graph,
    Diagram* dgrmOld,
    Diagram* dgrmNew )
// -----------------------
{
    TiXmlDocument doc( path.c_str() );

    if ( doc.LoadFile() == true )
    {
        TiXmlElement* curNode    = NULL;
        
        try
        {
            
            curNode = doc.FirstChildElement();
            
            if ( curNode != NULL )
            {
                parseDiagram(
                    graph,
                    dgrmOld,
                    dgrmNew,
                    curNode );
            }

            curNode    = NULL;
        }
        catch ( const string* msg )
        {
            curNode = NULL;

            string* errMsg = new string( "Error loading diagram." );
            
            errMsg->append( "\n" );
            errMsg->append( *msg );

            delete msg;
            msg = NULL;
            
            throw errMsg;
        }
    }
    else
    {
        string* msg = new string( "Error opening diagram file." );
        throw msg; 
    }
}


// -----------------------
void Parser::writeDiagram(
    const string &path,
    Graph* graph,
    Diagram* diagram )
// -----------------------
{
    try
	{
        // new xml document
        TiXmlDocument     doc;
        TiXmlDeclaration* decl;
        TiXmlElement*     dgrm;
        TiXmlElement*     file;
        TiXmlElement*     shpe;
        TiXmlElement*     prop;
        TiXmlElement*     subp;
        ColorRGB col;
        Attribute *       attr;

        // document declaration
        decl = new TiXmlDeclaration( "1.0", "", "" );  
	    doc.LinkEndChild( decl );

        // configuration
	    dgrm = new TiXmlElement( "Diagram" );  
	    doc.LinkEndChild( dgrm );
 
        // file name
	    file = new TiXmlElement( "File" );  
	    dgrm->LinkEndChild( file );
        file->LinkEndChild( new TiXmlText( graph->getFileName().c_str() ) );

        // shapes
        for ( int i = 0; i < diagram->getSizeShapes(); ++i )
        {
            shpe = new TiXmlElement( "Shape" );
            dgrm->LinkEndChild( shpe );

            // coordinates
            prop = new TiXmlElement( "XCenter" );
            shpe->LinkEndChild( prop );
            prop->LinkEndChild( 
                new TiXmlText(
                    Utils::dblToStr(
                        diagram->getShape(i)->getXCtr() ).c_str() ) );

            prop = new TiXmlElement( "YCenter" );
            shpe->LinkEndChild( prop );
            prop->LinkEndChild( 
                new TiXmlText(
                    Utils::dblToStr(
                        diagram->getShape(i)->getYCtr() ).c_str() ) );

            // distance from center
            prop = new TiXmlElement( "XDistanceFromCenter" );
            shpe->LinkEndChild( prop );
            prop->LinkEndChild( 
                new TiXmlText(
                    Utils::dblToStr(
                        diagram->getShape(i)->getXDFC() ).c_str() ) );

            prop = new TiXmlElement( "YDistanceFromCenter" );
            shpe->LinkEndChild( prop );
            prop->LinkEndChild( 
                new TiXmlText(
                    Utils::dblToStr(
                        diagram->getShape(i)->getYDFC() ).c_str() ) );

            // hinge
            prop = new TiXmlElement( "XHinge" );
            shpe->LinkEndChild( prop );
            prop->LinkEndChild( 
                new TiXmlText(
                    Utils::dblToStr(
                        diagram->getShape(i)->getXHinge() ).c_str() ) );

            prop = new TiXmlElement( "YHinge" );
            shpe->LinkEndChild( prop );
            prop->LinkEndChild( 
                new TiXmlText(
                    Utils::dblToStr(
                        diagram->getShape(i)->getYHinge() ).c_str() ) );

            // angle center
            prop = new TiXmlElement( "AngleCenter" );
            shpe->LinkEndChild( prop );
            prop->LinkEndChild( 
                new TiXmlText(
                    Utils::dblToStr(
                        diagram->getShape(i)->getAngleCtr() ).c_str() ) );

            // type
            prop = new TiXmlElement( "Type" );
            shpe->LinkEndChild( prop );
            if ( diagram->getShape(i)->getType() == Shape::TYPE_LINE )
            {
                prop->LinkEndChild( 
                    new TiXmlText( "TYPE_LINE" ) );
            }
            else if ( diagram->getShape(i)->getType() == Shape::TYPE_RECT )
            {
                prop->LinkEndChild( 
                    new TiXmlText( "TYPE_RECT" ) );
            }
            else if ( diagram->getShape(i)->getType() == Shape::TYPE_ELLIPSE )
            {
                prop->LinkEndChild( 
                    new TiXmlText( "TYPE_ELLIPSE" ) );
            }
            else if ( diagram->getShape(i)->getType() == Shape::TYPE_ARROW )
            {
                prop->LinkEndChild( 
                    new TiXmlText( "TYPE_ARROW" ) );
            }
            else if ( diagram->getShape(i)->getType() == Shape::TYPE_DARROW )
            {
                prop->LinkEndChild( 
                    new TiXmlText( "TYPE_DARROW" ) );
            }
            
            // line width
            prop = new TiXmlElement( "LineWidth" );
            shpe->LinkEndChild( prop );
            prop->LinkEndChild( 
                new TiXmlText(
                    Utils::dblToStr(
                        diagram->getShape(i)->getLineWidth() ).c_str() ) );
            
            // color line
            prop = new TiXmlElement( "LineColor" );
            shpe->LinkEndChild( prop );
            
            diagram->getShape(i)->getLineColor( col );
            subp = new TiXmlElement( "Red" );
            prop->LinkEndChild( subp );
            subp->LinkEndChild(
                new TiXmlText( 
                    Utils::dblToStr( col.r ).c_str() ) );
            subp = new TiXmlElement( "Green" );
            prop->LinkEndChild( subp );
            subp->LinkEndChild(
                new TiXmlText( 
                    Utils::dblToStr( col.g ).c_str() ) );
            subp = new TiXmlElement( "Blue" );
            prop->LinkEndChild( subp );
            subp->LinkEndChild(
                new TiXmlText( 
                    Utils::dblToStr( col.b ).c_str() ) );
            subp = new TiXmlElement( "Alpha" );
            prop->LinkEndChild( subp );
            subp->LinkEndChild(
                new TiXmlText( 
                    Utils::dblToStr( col.a ).c_str() ) );

            // color fill
            prop = new TiXmlElement( "FillColor" );
            shpe->LinkEndChild( prop );
            
            diagram->getShape(i)->getFillColor( col );
            subp = new TiXmlElement( "Red" );
            prop->LinkEndChild( subp );
            subp->LinkEndChild(
                new TiXmlText( 
                    Utils::dblToStr( col.r ).c_str() ) );
            subp = new TiXmlElement( "Green" );
            prop->LinkEndChild( subp );
            subp->LinkEndChild(
                new TiXmlText( 
                    Utils::dblToStr( col.g ).c_str() ) );
            subp = new TiXmlElement( "Blue" );
            prop->LinkEndChild( subp );
            subp->LinkEndChild(
                new TiXmlText( 
                    Utils::dblToStr( col.b ).c_str() ) );
            subp = new TiXmlElement( "Alpha" );
            prop->LinkEndChild( subp );
            subp->LinkEndChild(
                new TiXmlText( 
                    Utils::dblToStr( col.a ).c_str() ) );
        
            // X center DOF
            prop = new TiXmlElement( "XCenterDOF" );
            shpe->LinkEndChild( prop );

            subp = new TiXmlElement( "Attribute" );
            prop->LinkEndChild( subp );
            attr = diagram->getShape(i)->getDOFXCtr()->getAttribute();
            if ( attr != NULL )
                subp->LinkEndChild(
                    new TiXmlText( 
                        attr->getName().c_str() ) );
            attr = NULL;

            {
            for ( int j = 0; j < diagram->getShape(i)->getDOFXCtr()->getSizeValues(); ++j )
            {
                subp = new TiXmlElement( "Value" );
                prop->LinkEndChild( subp );
                subp->LinkEndChild(
                    new TiXmlText( 
                        Utils::dblToStr( 
                            diagram->getShape(i)->getDOFXCtr()->getValue(j) ).c_str() ) );    
            }
            }

            // Y center DOF
            prop = new TiXmlElement( "YCenterDOF" );
            shpe->LinkEndChild( prop );

            subp = new TiXmlElement( "Attribute" );
            prop->LinkEndChild( subp );
            attr = diagram->getShape(i)->getDOFYCtr()->getAttribute();
            if ( attr != NULL )
                subp->LinkEndChild(
                    new TiXmlText( 
                        attr->getName().c_str() ) );
            attr = NULL;

            {
            for ( int j = 0; j < diagram->getShape(i)->getDOFYCtr()->getSizeValues(); ++j )
            {
                subp = new TiXmlElement( "Value" );
                prop->LinkEndChild( subp );
                subp->LinkEndChild(
                    new TiXmlText( 
                        Utils::dblToStr( 
                            diagram->getShape(i)->getDOFYCtr()->getValue(j) ).c_str() ) );    
            }
            }

            // width DOF
            prop = new TiXmlElement( "WidthDOF" );
            shpe->LinkEndChild( prop );

            subp = new TiXmlElement( "Attribute" );
            prop->LinkEndChild( subp );
            attr = diagram->getShape(i)->getDOFWth()->getAttribute();
            if ( attr != NULL )
                subp->LinkEndChild(
                    new TiXmlText( 
                        attr->getName().c_str() ) );
            attr = NULL;
            
            {
            for ( int j = 0; j < diagram->getShape(i)->getDOFWth()->getSizeValues(); ++j )
            {
                subp = new TiXmlElement( "Value" );
                prop->LinkEndChild( subp );
                subp->LinkEndChild(
                    new TiXmlText( 
                        Utils::dblToStr( 
                            diagram->getShape(i)->getDOFWth()->getValue(j) ).c_str() ) );    
            }
            }
            
            // height DOF
            prop = new TiXmlElement( "HeightDOF" );
            shpe->LinkEndChild( prop );

            subp = new TiXmlElement( "Attribute" );
            prop->LinkEndChild( subp );
            attr = diagram->getShape(i)->getDOFHgt()->getAttribute();
            if ( attr != NULL )
                subp->LinkEndChild(
                    new TiXmlText( 
                        attr->getName().c_str() ) );
            attr = NULL;

            prop->LinkEndChild( subp );
            {
            for ( int j = 0; j < diagram->getShape(i)->getDOFHgt()->getSizeValues(); ++j )
            {
                subp = new TiXmlElement( "Value" );
                prop->LinkEndChild( subp );
                subp->LinkEndChild(
                    new TiXmlText( 
                        Utils::dblToStr( 
                            diagram->getShape(i)->getDOFHgt()->getValue(j) ).c_str() ) );    
            }
            }

            // angle DOF (relative to hinge)
            prop = new TiXmlElement( "AngleDOF" );
            shpe->LinkEndChild( prop );

            subp = new TiXmlElement( "Attribute" );
            prop->LinkEndChild( subp );
            attr = diagram->getShape(i)->getDOFAgl()->getAttribute();
            if ( attr != NULL )
                subp->LinkEndChild(
                    new TiXmlText( 
                        attr->getName().c_str() ) );
            attr = NULL;

            prop->LinkEndChild( subp );
            {
            for ( int j = 0; j < diagram->getShape(i)->getDOFAgl()->getSizeValues(); ++j )
            {
                subp = new TiXmlElement( "Value" );
                prop->LinkEndChild( subp );
                subp->LinkEndChild(
                    new TiXmlText( 
                        Utils::dblToStr( 
                            diagram->getShape(i)->getDOFAgl()->getValue(j) ).c_str() ) );    
            }
            }

            // color DOF
            prop = new TiXmlElement( "ColorDOF" );
            shpe->LinkEndChild( prop );

            subp = new TiXmlElement( "Attribute" );
            prop->LinkEndChild( subp );
            attr = diagram->getShape(i)->getDOFCol()->getAttribute();
            if ( attr != NULL )
                subp->LinkEndChild(
                    new TiXmlText( 
                        attr->getName().c_str() ) );
            attr = NULL;

            {
            for ( int j = 0; j < diagram->getShape(i)->getDOFCol()->getSizeValues(); ++j )
            {
                subp = new TiXmlElement( "Value" );
                prop->LinkEndChild( subp );
                subp->LinkEndChild(
                    new TiXmlText( 
                        Utils::dblToStr( 
                            diagram->getShape(i)->getDOFCol()->getValue(j) ).c_str() ) );    
            }
            }

            vector< double > yValsCol;
            diagram->getShape(i)->getDOFColYValues( yValsCol );
            {
            for ( size_t j = 0; j < yValsCol.size(); ++j )
            {
                subp = new TiXmlElement( "AuxilaryValue" );
                prop->LinkEndChild( subp );
                subp->LinkEndChild(
                    new TiXmlText( 
                        Utils::dblToStr( 
                            yValsCol[j] ).c_str() ) );    
            }
            }

            // opacity DOF
            prop = new TiXmlElement( "OpacityDOF" );
            shpe->LinkEndChild( prop );

            subp = new TiXmlElement( "Attribute" );
            prop->LinkEndChild( subp );
            attr = diagram->getShape(i)->getDOFOpa()->getAttribute();
            if ( attr != NULL )
                subp->LinkEndChild(
                    new TiXmlText( 
                        attr->getName().c_str() ) );
            attr = NULL;

            {
            for ( int j = 0; j < diagram->getShape(i)->getDOFOpa()->getSizeValues(); ++j )
            {
                subp = new TiXmlElement( "Value" );
                prop->LinkEndChild( subp );
                subp->LinkEndChild(
                    new TiXmlText( 
                        Utils::dblToStr( 
                            diagram->getShape(i)->getDOFOpa()->getValue(j) ).c_str() ) );    
            }
            }

            vector< double > yValsOpa;
            diagram->getShape(i)->getDOFOpaYValues( yValsOpa );
            {
            for ( size_t j = 0; j < yValsOpa.size(); ++j )
            {
                subp = new TiXmlElement( "AuxilaryValue" );
                prop->LinkEndChild( subp );
                subp->LinkEndChild(
                    new TiXmlText( 
                        Utils::dblToStr( 
                            yValsOpa[j] ).c_str() ) );    
            }
            }
        }

        doc.SaveFile( path.c_str() );
        
        decl = NULL;
        dgrm = NULL;
        file = NULL;
        shpe = NULL;
        prop = NULL;
        subp = NULL;
        attr = NULL;
    }
    catch ( ... )
    {
        string* msg = new string(
            "Error saving diagram." );
        throw msg; 
    }
}
    

// -- private utility functions -------------------------------------


// -----------------------------
void Parser::parseStateVarDescr( 
    const string &nextLine,
    Graph* graph )
// -----------------------------
// ------------------------------------------------------------------
// This function is used by Parser::parseFsmFile() to parse the state
// description (first) part of an fsm file.
// ------------------------------------------------------------------
{
    string token;
    string name;
    string card;
    string type;
    vector< string > values;
    try
    {
        string::size_type begIdx;
        string::size_type endIdx;

        // description
        begIdx = nextLine.find_first_not_of( delims );
        endIdx = nextLine.find_first_of( delims );
        token = nextLine.substr( begIdx, endIdx-begIdx );
        if ( token == "\n" )
            name = "unspecified";
        else
            name = token;
        
        // cardinality
        begIdx = nextLine.find_first_not_of( delims, endIdx );
        endIdx = nextLine.find_first_of( delims, begIdx );
        token = nextLine.substr( begIdx, endIdx-begIdx );
        card = token;
        
        // type
        begIdx = nextLine.find_first_not_of( "() ", endIdx );
        endIdx = nextLine.find_first_of( "() ", begIdx );
        token = nextLine.substr( begIdx, endIdx-begIdx );
        if ( token == "\n" )
            // end of line
            type = "Unspecified";
        else if ( token.substr( 0, 1 ) == "\"" )
        {
            // no type specified
            type = "Unspecified";
            endIdx = begIdx;
        }
        else
            type = token;
        
        // domain
        if ( card != "0" )
        {
            // all domain values are specified
            begIdx = nextLine.find_first_not_of( " \"", endIdx );
            endIdx = nextLine.find_first_of( "\"", begIdx );
        
            while ( endIdx != string::npos )
            {
                token = nextLine.substr( begIdx, endIdx-begIdx );
                if( token != "\n" )
                      values.push_back( token );
                begIdx = nextLine.find_first_not_of( " \"", endIdx );
                endIdx = nextLine.find_first_of( "\"", begIdx );
            }

            // add new discrete attribute to graph
            graph->addAttrDiscr(
                name,
                type,
                graph->getSizeAttributes(),
                values );
        }
        else
        {
            // range of domain values are specified
            double lwrBnd = 0.0;
            double uprBnd = 0.0;

            // lower bound
            begIdx = nextLine.find_first_not_of( " [", endIdx );
            endIdx = nextLine.find_first_of( " ,", begIdx );
            if ( endIdx != string::npos )
            {
                token  = nextLine.substr( begIdx, endIdx-begIdx );
                lwrBnd = Utils::strToDbl( token );

                // upper bound
                begIdx = nextLine.find_first_not_of( " ,", endIdx );
                endIdx = nextLine.find_first_of( "]", begIdx );
                if ( endIdx != string::npos )
                {
                    token  = nextLine.substr( begIdx, endIdx-begIdx );
                    uprBnd = Utils::strToDbl( token );
                }
            }

            // add new continuous attribute to graph
            graph->addAttrConti(
                name,
                type,
                graph->getSizeAttributes(),
                lwrBnd,
                uprBnd );
        }
    }
    catch ( ... )
    {
        string* msg = new string(
            "Error parsing state description." );
        throw msg;
    }

}


// ------------------------
void Parser::parseStates( 
    const string &nextLine,
    Graph* graph )
// ------------------------
// -----------------------------------------------------------------------
// This function is used by Parser::ParseFsmFile() to parse the states
// (second) part of an fsm file.
// -----------------------------------------------------------------------
{
    vector< double > stateVector;
    try
    {
        string token = "";
        string::size_type begIdx;
        string::size_type endIdx;

        begIdx = nextLine.find_first_not_of( delims );
        endIdx = nextLine.find_first_of( delims );
        // get all tokens in line
        while ( begIdx != string::npos )
        {
            token = nextLine.substr( begIdx, endIdx-begIdx );
            if( token != "\n" )
                stateVector.push_back( Utils::strToDbl( token ) );
            begIdx = nextLine.find_first_not_of( delims, endIdx );
            endIdx = nextLine.find_first_of( delims, begIdx );
        }

        // add new node to graph
        graph->addNode( stateVector );
    }
    catch ( ... )
    {
        throw new string( "Error parsing states." );
    }
}


// ---------------------------
void Parser::parseTransitions( 
    const int &lineNumber,
    const string &nextLine,
    Graph* graph )
// ---------------------------
// -----------------------------------------------------------------------
// This function is used by Parser::ParseFsmFile() to parse the 
// transitions (third) part of an fsm file.
// -----------------------------------------------------------------------
{
    int    frStateIdx;
    int    toStateIdx;
    string trLbl;
    //vector< string > params;
    try
    {
        string token    = "";
        string::size_type begIdx;
        string::size_type endIdx;

        // index of from state
        begIdx     = nextLine.find_first_not_of( delims );
        endIdx     = nextLine.find_first_of( delims );
        token      = nextLine.substr( begIdx, endIdx-begIdx );
        frStateIdx = Utils::strToInt( token ) - 1;

        // index (1-based) of to state
        begIdx     = nextLine.find_first_not_of( delims, endIdx );
        endIdx     = nextLine.find_first_of( delims, begIdx );
        token      = nextLine.substr( begIdx, endIdx-begIdx );
        toStateIdx = Utils::strToInt( token ) - 1;

        // label of transition
        begIdx = nextLine.find_first_not_of( delims, endIdx );
        string lblDelims = "()\"";
        endIdx = nextLine.find_first_of( lblDelims, begIdx );
        if ( endIdx != string::npos )
            token = nextLine.substr( begIdx, endIdx-begIdx );
        else
            token = "";
        trLbl = token;
        /*
        // transition parameters
        if ( endIdx != string::npos )
        {
            begIdx = nextLine.find_first_not_of( delims, endIdx );
            endIdx = nextLine.find_last_of( ")" );
            token = nextLine.substr( begIdx, endIdx-begIdx );
        }
        else
            token = "";
        if ( token != "\n" )
            params.push_back( token );
        */
        graph->addEdge( 
            trLbl,
            frStateIdx,
            toStateIdx );
    }
    catch ( ... )
    {
        throw new string( "Error parsing transitions." );
    }
}


// ---------------------------------------------------
void Parser::parseAttrConfig(
    Graph* graph,
    map< int, int > &attrIdxFrTo,
    map< int, vector< string > > &attrCurDomains,
    map< int, map< int, int  > > &attrOrigToCurDomains,
    TiXmlElement* curNode )
// ---------------------------------------------------
{
    if ( curNode != NULL && curNode->Value() != NULL)
    {
        // file
        if ( strcmp( curNode->Value(), "File") == 0 )
        {
            /*
            // the code below checks for matching file names
            if ( curNode->FirstChild()->ToText()->Value() != NULL )
            {
                if ( strcmp( curNode->FirstChild()->ToText()->Value(),
                     graph->getFileName().c_str() ) != 0 )
                {
                    throw new string( "File names do not match." );
                }
            }
            else
            {
                throw new string( "No file name specified." );
            }
            */

            // the code below does not check for matching file names
            if ( curNode->FirstChild()->ToText()->Value() == NULL )
                throw new string( "No file name specified." );
        }
        // shape
        else if ( strcmp( curNode->Value(), "Attribute") == 0)
        {
            try
            {
                parseAttr( 
                    graph, 
                    attrIdxFrTo, 
                    attrCurDomains, 
                    attrOrigToCurDomains, 
                    curNode );
            }
            catch ( const string* msg )
            {
                string* errMsg = new string( "Error parsing attribute." );
            
                errMsg->append( "\n" );
                errMsg->append( *msg );

                delete msg;
                msg = NULL;
            
                throw errMsg;
            }
        }
        // other
        else
        {
            TiXmlElement* nxtNode;
            for ( nxtNode = curNode->FirstChildElement(); 
                  nxtNode != NULL;
                  nxtNode = nxtNode->NextSiblingElement() )
            {
                parseAttrConfig( 
                    graph, 
                    attrIdxFrTo, 
                    attrCurDomains, 
                    attrOrigToCurDomains,
                    nxtNode );
            }
            nxtNode = NULL;
        }
    }
}


// ----------------------------------------------------
void Parser::parseAttr(
    Graph* graph,
    map< int, int > &attrIdxFrTo,
    map< int, vector< string > > &attrCurDomains,
    map< int, map< int, int  > > &attrOrigToCurDomains,
    TiXmlElement* curNode )
// ----------------------------------------------------
{
    TiXmlNode* prop = NULL;
    TiXmlNode* subp = NULL;
    TiXmlNode* ssbp = NULL;

    Attribute* attr;

    // name
    prop = curNode->FirstChild();
    if ( prop != NULL &&
         strcmp( prop->Value(), "Name" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
        {
            attr = graph->getAttribute( subp->ToText()->Value() );
            if ( attr == NULL )
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Missing attribute." );
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing attribute." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing attribute." );
    }

    // type
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "Type" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
        {
            if ( strcmp( subp->ToText()->Value(), attr->getType().c_str() ) != 0 )
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Types do not match." ); 
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing type." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing type." );
    }

    // card
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "OriginalCardinality" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
        {
            if ( strcmp( subp->ToText()->Value(), Utils::intToStr( attr->getSizeOrigValues() ).c_str() ) != 0 )
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Cardinalities do not match." ); 
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing cardinality." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing cardinality." );
    }

    // update attribute mapping
    attrIdxFrTo.insert( pair< int, int >( attr->getIndex(), attrIdxFrTo.size() ) );

    // current domain
    vector< string > domain;
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "CurrentDomain" ) == 0 )
    {
        for ( subp = prop->FirstChild(); subp != NULL; subp = subp->NextSibling() )
        {
            if ( strcmp( subp->Value(), "Value" ) == 0 )
            {
                ssbp = subp->FirstChild();

                if ( ssbp != NULL )
                    domain.push_back( ssbp->ToText()->Value() );
                else
                {
                    prop = NULL;
                    subp = NULL;
                    ssbp = NULL;
                    throw new string( "Missing domain value." );
                }
            }
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Domain incorrectly specified." );
            }
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing domain." );
    }
    
    // update domain
    attrCurDomains.insert( pair< int, vector< string > >( attr->getIndex(), domain ) );

    // mapping from orig to current domain values
    map< int, int > origToCur;
    int valCnt = 0;
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "OriginalToCurrent" ) == 0 )
    {
        for ( subp = prop->FirstChild(); subp != NULL; subp = subp->NextSibling() )
        {
            if ( strcmp( subp->Value(), "CurrentPosition" ) == 0 )
            {
                ssbp = subp->FirstChild();

                if ( ssbp != NULL )
                {
                    origToCur.insert( 
                        pair< int, int >( 
                        valCnt, Utils::strToInt( ssbp->ToText()->Value() ) ) );
                    ++valCnt;
                }
                else
                {
                    prop = NULL;
                    subp = NULL;
                    ssbp = NULL;
                    throw new string( "Missing mapping from original to current domain." );
                }
            }
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Mapping from original to current domain incorrectly specified." );
            }
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing mapping from original to current domain." );
    }

    // update mapping
    attrOrigToCurDomains.insert( pair< int, map< int, int > >( attr->getIndex(), origToCur ) );

    attr = NULL;
}


// ------------------------
void Parser::parseDiagram(
    Graph* graph,
    Diagram* dgrmOld,
    Diagram* dgrmNew,
    TiXmlElement* curNode )
// ------------------------   
{
    if ( curNode != NULL && curNode->Value() != NULL)
    {
        // file
        if ( strcmp( curNode->Value(), "File") == 0 )
        {
            /* 
            // the code below checks for matching file names
            if ( curNode->FirstChild()->ToText()->Value() != NULL )
            {
                if ( strcmp( curNode->FirstChild()->ToText()->Value(),
                     graph->getFileName().c_str() ) != 0 )
                {
                    throw new string( "File names do not match." );
                }
            }
            else
            {
                throw new string( "No file name specified." );
            }
            */

            // the code below does not check for matchin file names
            if ( curNode->FirstChild()->ToText()->Value() == NULL )
                throw new string( "No file name specified." );
        }
        // shape
        else if ( strcmp( curNode->Value(), "Shape") == 0)
        {
            try
            {
                parseShape( graph, dgrmOld, dgrmNew, curNode );
            }
            catch ( const string* msg )
            {
                string* errMsg = new string( "Error parsing shape." );
            
                errMsg->append( "\n" );
                errMsg->append( *msg );

                delete msg;
                msg = NULL;
            
                throw errMsg;
            }
        }
        // other
        else
        {
            TiXmlElement* nxtNode;
            for ( nxtNode = curNode->FirstChildElement(); 
                  nxtNode != NULL;
                  nxtNode = nxtNode->NextSiblingElement() )
            {
                parseDiagram( graph, dgrmOld, dgrmNew, nxtNode );
            }
            nxtNode = NULL;
        }
    }
}


// ------------------------
void Parser::parseShape(
    Graph* graph,
    Diagram* dgrmOld,
    Diagram* dgrmNew,
    TiXmlElement* curNode )
// ------------------------
{
    TiXmlNode* prop = NULL;
    TiXmlNode* subp = NULL;
    TiXmlNode* ssbp = NULL;

    double xCtr, yCtr;
    double xDFC, yDFC;
    double xHge, yHge;
    double aglCtr;

    int      type;
    double   lineWth;
    ColorRGB lineCol;
    ColorRGB fillCol;

    // x center
    prop = curNode->FirstChild();
    if ( prop != NULL &&
         strcmp( prop->Value(), "XCenter" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
            xCtr = Utils::strToDbl( subp->ToText()->Value() );
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing x-coordinate." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing x-coordinate." );
    }

    // y center
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "YCenter" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
            yCtr = Utils::strToDbl( subp->ToText()->Value() );
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing y-coordinate." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing y-coordinate." );
    }
        
    // x distance from center
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "XDistanceFromCenter" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
             xDFC = Utils::strToDbl( subp->ToText()->Value() );
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing x distance from center." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing x distance from center." );
    }
        
    // y distance from center
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "YDistanceFromCenter" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
             yDFC = Utils::strToDbl( subp->ToText()->Value() );
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing y distance from center." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing y distance from center." );
    }
        
    // x hinge
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "XHinge" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
             xHge = Utils::strToDbl( subp->ToText()->Value() );
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing x hinge." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing x hinge." );
    }
        
    // y hinge
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "YHinge" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
            yHge = Utils::strToDbl( subp->ToText()->Value() );
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing y hinge." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing y hinge." );
    }
        
    // angle center
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "AngleCenter" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
            aglCtr = Utils::strToDbl( subp->ToText()->Value() );
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing angle." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing angle." );
    }
        
    // type
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "Type" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
        {
            if ( strcmp( subp->ToText()->Value(), "TYPE_LINE" ) == 0 )
                type = Shape::TYPE_LINE;
            else if ( strcmp( subp->ToText()->Value(), "TYPE_RECT" ) == 0 )
                type = Shape::TYPE_RECT;
            else if ( strcmp( subp->ToText()->Value(), "TYPE_ELLIPSE" ) == 0 )
                type = Shape::TYPE_ELLIPSE;
            else if ( strcmp( subp->ToText()->Value(), "TYPE_ARROW" ) == 0 )
                type = Shape::TYPE_ARROW;
            else if ( strcmp( subp->ToText()->Value(), "TYPE_DARROW" ) == 0 )
                type = Shape::TYPE_DARROW;
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing type." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing type." );
    }
    
    // line width
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "LineWidth" ) == 0 )
    {
        subp = prop->FirstChild();
        if ( subp != NULL )
            lineWth = Utils::strToDbl( subp->ToText()->Value() );
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing line width." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing line width." );
    }

    // line color
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "LineColor" ) == 0 )
    {
        // red
        subp = prop->FirstChild();
        if ( subp != NULL && 
             strcmp( subp->Value(), "Red" ) == 0 )
        {
            ssbp = subp->FirstChild();
            if ( ssbp != NULL )
                lineCol.r = Utils::strToDbl( ssbp->ToText()->Value() ); 
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Missing red channel." );
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing red channel." );
        }

        // green
        subp = subp->NextSibling();
        if ( subp != NULL && 
             strcmp( subp->Value(), "Green" ) == 0 )
        {
            ssbp = subp->FirstChild();
            if ( ssbp != NULL )
                lineCol.g = Utils::strToDbl( ssbp->ToText()->Value() );
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Missing green channel." );
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing green color." );
        }

        // blue
        subp = subp->NextSibling();
        if ( subp != NULL && 
             strcmp( subp->Value(), "Blue" ) == 0 )
        {
            ssbp = subp->FirstChild();
            if ( ssbp != NULL )
                lineCol.b = Utils::strToDbl( ssbp->ToText()->Value() );
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Missing blue channel." );
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing blue color." );
        }

        // alpha
        subp = subp->NextSibling();
        if ( subp != NULL && 
             strcmp( subp->Value(), "Alpha" ) == 0 )
        {
            ssbp = subp->FirstChild();
            if ( ssbp != NULL )
                lineCol.a = Utils::strToDbl( ssbp->ToText()->Value() );
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Missing alpha channel." );
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing alpha color." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing line color." );
    }

    // fill color
    prop = prop->NextSibling();
    if ( prop != NULL &&
         strcmp( prop->Value(), "FillColor" ) == 0 )
    {
        // red
        subp = prop->FirstChild();
        if ( subp != NULL && 
             strcmp( subp->Value(), "Red" ) == 0 )
        {
            ssbp = subp->FirstChild();
            if ( ssbp != NULL )
                fillCol.r = Utils::strToDbl( ssbp->ToText()->Value() ); 
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Missing red channel." );
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing red channel." );
        }

        // green
        subp = subp->NextSibling();
        if ( subp != NULL && 
             strcmp( subp->Value(), "Green" ) == 0 )
        {
            ssbp = subp->FirstChild();
            if ( ssbp != NULL )
                fillCol.g = Utils::strToDbl( ssbp->ToText()->Value() );
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Missing green channel." );
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing green color." );
        }

        // blue
        subp = subp->NextSibling();
        if ( subp != NULL && 
             strcmp( subp->Value(), "Blue" ) == 0 )
        {
            ssbp = subp->FirstChild();
            if ( ssbp != NULL )
                fillCol.b = Utils::strToDbl( ssbp->ToText()->Value() );
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Missing blue channel." );
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing blue color." );
        }

        // alpha
        subp = subp->NextSibling();
        if ( subp != NULL && 
             strcmp( subp->Value(), "Alpha" ) == 0 )
        {
            ssbp = subp->FirstChild();
            if ( ssbp != NULL )
                fillCol.a = Utils::strToDbl( ssbp->ToText()->Value() );
            else
            {
                prop = NULL;
                subp = NULL;
                ssbp = NULL;
                throw new string( "Missing alpha channel." );
            }
        }
        else
        {
            prop = NULL;
            subp = NULL;
            ssbp = NULL;
            throw new string( "Missing alpha channel." );
        }
    }
    else
    {
        prop = NULL;
        subp = NULL;
        ssbp = NULL;
        throw new string( "Missing fill color." );
    }

    // init shape
    Shape* s = new Shape(
        mediator,
        dgrmNew->getSizeShapes(), 
        xCtr,   yCtr,
        xDFC,   yDFC,
        aglCtr, type );
    s->setLineWidth( lineWth );
    s->setLineColor( lineCol );
    s->setFillColor( fillCol );

    // DOF's
    Attribute* attr;
    DOF*       dof;
    
    int  cntVal;
    int  cntAuxCol;
    int  cntAuxOpa;
    
    for ( prop = prop->NextSibling(); prop != NULL; prop = prop->NextSibling() )
    {
        attr = NULL;
        dof  = NULL;

        if ( prop != NULL )
        {
            // dof
            if ( strcmp( prop->Value(), "XCenterDOF" ) == 0 )
                dof = s->getDOFXCtr();
            else if ( strcmp( prop->Value(), "YCenterDOF" ) == 0 )
                dof = s->getDOFYCtr();
            else if ( strcmp( prop->Value(), "WidthDOF" ) == 0 )
                dof = s->getDOFWth();
            else if ( strcmp( prop->Value(), "HeightDOF" ) == 0 )
                dof = s->getDOFHgt();
            else if ( strcmp( prop->Value(), "AngleDOF" ) == 0 )
                dof = s->getDOFAgl();
            else if ( strcmp( prop->Value(), "ColorDOF" ) == 0 )
                dof = s->getDOFCol();
            else if ( strcmp( prop->Value(), "OpacityDOF" ) == 0 )
                dof = s->getDOFOpa();
            
            if ( dof != NULL )
            {
                // attribute
                subp = prop->FirstChild();
                if ( subp != NULL &&
                     strcmp( subp->Value(), "Attribute" ) == 0 )
                {
                    ssbp = subp->FirstChild();
                    if ( ssbp != NULL )
                    {
                        attr = graph->getAttribute( ssbp->ToText()->Value() );
                        if ( attr != NULL )
                            dof->setAttribute( attr );
                    }
                }

                // values
                cntVal    = 0;
                cntAuxCol = 0;
                cntAuxOpa = 0;

                for ( subp = prop->FirstChild(); subp != NULL; subp = subp->NextSibling() )
                {
                    if ( strcmp( subp->Value(), "Value" ) == 0 )
                    {
                        ssbp = subp->FirstChild();
                        if ( ssbp != NULL )
                        {
                            if ( ssbp->ToText()->Value() != NULL )
                            {
                                ++cntVal;

                                // reset min
                                if ( cntVal == 1 )
                                    dof->setMin( Utils::strToDbl( ssbp->ToText()->Value() ) );
                                // reset max
                                else if ( cntVal == 2 )
                                    dof->setMax( Utils::strToDbl( ssbp->ToText()->Value() ) );
                                // add additional values
                                else
                                    dof->addValue( Utils::strToDbl( ssbp->ToText()->Value() ) );
                            }
                        }
                    }
                    else if ( strcmp( subp->Value(), "AuxilaryValue" ) == 0 &&
                              strcmp( prop->Value(), "ColorDOF" ) == 0 )
                    {
                        ssbp = subp->FirstChild();
                        if ( ssbp != NULL )
                        {
                            if ( ssbp->ToText()->Value() != NULL )
                            {
                                ++cntAuxCol;

                                // update first value
                                if ( cntAuxCol == 1 )
                                    s->setDOFColYValue( 0, Utils::strToDbl( ssbp->ToText()->Value() ) );
                                // update second value
                                else if ( cntAuxCol == 2 )
                                    s->setDOFColYValue( 1, Utils::strToDbl( ssbp->ToText()->Value() ) );
                                // add additional values
                                else
                                    s->addDOFColYValue( Utils::strToDbl( ssbp->ToText()->Value() ) );
                            }
                        }
                    }
                    else if ( strcmp( subp->Value(), "AuxilaryValue" ) == 0 &&
                              strcmp( prop->Value(), "OpacityDOF" ) == 0 )
                    {
                        ssbp = subp->FirstChild();
                        if ( ssbp != NULL )
                        {
                            if ( ssbp->ToText()->Value() != NULL )
                            {
                                ++cntAuxOpa;

                                // update first value
                                if ( cntAuxOpa == 1 )
                                    s->setDOFOpaYValue( 0, Utils::strToDbl( ssbp->ToText()->Value() ) );
                                // update second value
                                else if ( cntAuxOpa == 2 )
                                    s->setDOFOpaYValue( 1, Utils::strToDbl( ssbp->ToText()->Value() ) );
                                // add additional values
                                else
                                    s->addDOFOpaYValue( Utils::strToDbl( ssbp->ToText()->Value() ) );
                            }
                        }
                    }
                }

            } // dof
        } // prop
    
    }

    // add shape
    dgrmNew->addShape( s );
    s =NULL;

    attr = NULL;
    dof  = NULL;

    prop = NULL;
    subp = NULL;
    ssbp = NULL;
}


// -- end -----------------------------------------------------------
