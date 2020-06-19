/**
 *  Orginally from:
 *    https://embeddedartistry.com/blog/2017/05/17/creating-a-circular-buffer-in-c-and-c/
 *    https://github.com/embeddedartistry/embedded-resources/blob/master/examples/cpp/circular_buffer.cpp
 *
 *  Made into stack instead of queue, added reading prev/next while not pop/push
 *  and no pop push while reading.
 */

#include "Scintilla.h"
#include "CircularStackLinkList.h"

circular_buffer<int> prevPos( STACK_SIZE );
circular_buffer<int> nextPos( STACK_SIZE );

int getCurrentPos()
{
    Sci_Position currentPos = ( Sci_Position )::SendMessage( getCurScintilla(),
                              SCI_GETCURRENTPOS, 0, 0 );
    int currentLine = ( int )::SendMessage( getCurScintilla(),
                              SCI_LINEFROMPOSITION, currentPos, 0 );
    return currentLine;
}

void gotoNewPos( int gotoPos )
{
    ::SendMessage( getCurScintilla(), SCI_GOTOLINE, gotoPos, 0 );
}

void gotoPrevPos()
{
    if ( ! prevPos.empty() )
    {
        nextPos.put( getCurrentPos() );
        gotoNewPos( prevPos.get() );
    }
}

void gotoNextPos()
{
    if ( ! nextPos.empty() )
    {
        prevPos.put( getCurrentPos() );
        gotoNewPos( nextPos.get() );
    }
}
