#ifndef THUMBSSPREAD_H
#define THUMBSSPREAD_H

namespace WTS {

/*
 * 'anchers' are desired item-center positions
 * 'result' contains X coordinates of spread out item's left sides
 */
template<class Array>
void spread( int total_width, int item_width, const Array& anchers, Array& result )
{
    // initially should be at their desired positions
    foreach( int cx, anchers ) {
        result << std::max(0, std::min( total_width - item_width, cx - item_width/2));
    }
}

}

#endif // THUMBSSPREAD_H
