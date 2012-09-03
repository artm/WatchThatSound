#ifndef THUMBSSPREAD_H
#define THUMBSSPREAD_H

namespace WTS {

namespace detail {
struct Gap;

struct Item {
    Gap * left, * right;
    int anchor, pos;

    Item() : left(0), right(0), anchor(0), pos(0) {}
};

struct Gap {
    Item * left, * right;
    int pressure;

    Gap() : left(0), right(0), pressure(0) {}
};

struct Spread {
    int N, total_width, item_width, max_pressure;
    std::vector<Item> items;
    std::vector<Gap> gaps;

    template<class Array>
    Spread( int _total_width, int _item_width, const Array& anchors )
        : N(anchors.size()), total_width(_total_width), item_width(_item_width)
        , max_pressure( std::max(0, N*item_width - total_width) )
        , items(N), gaps(N)
    {
        for(int i = 0; i<N; i++) {
            items[i].anchor = items[i].pos = anchors[i];
        }
        for(int i = 0; i<N+1; i++) {
            if (i>0) {
                gaps[i].left = &items[i-1];
                items[i-1].right = &gaps[i];
            }
            if (i<N) {
                gaps[i].right = &items[i];
                items[i].left = &gaps[i];
            }
        }
        qDebug() << "Max pressure" << max_pressure;
    }

    void apply_hard_limits()
    {
        for(int i = 0; i<N; i++)
            items[i].pos = std::max( item_width/2, std::min( items[i].pos, total_width-item_width/2) );
    }

    int measure_gaps()
    {
        apply_hard_limits();
        int total = 0;
        // measure gaps
        for(int i = 0; i<N+1; i++) {
            int x1 = gaps[i].left ? gaps[i].left->pos + item_width/2 : 0;
            int x2 = gaps[i].right ? gaps[i].right->pos - item_width/2 : total_width;
            total += gaps[i].pressure = std::max( 0, x1 - x2 );
        }
        return total;
    }

    void run(double k)
    {
        int tot;
        while( (tot=measure_gaps()) > max_pressure ) {
            qDebug() << "total pressure" << tot;
            for(int i=0; i<N; i++) {
                int pressure = items[i].left->pressure - items[i].right->pressure;
                items[i].pos = floor( 0.5 + k * pressure + items[i].pos );
            }
        }
    }
};

}

/*
 * 'anchors' are desired item-center positions
 * 'result' contains X coordinates of spread out item's left sides
 */
template<class Array>
void spread( int total_width, int item_width, const Array& anchors, Array& result )
{
    foreach( int cx, anchors ) {
        result << std::max(0, std::min( total_width - item_width, cx - item_width/2));
    }

    /*
    detail::Spread spreader( total_width, item_width, anchors );
    spreader.run(0.5);
    for(int i = 0; i<spreader.N; i++) {
        int x = spreader.items[i].pos - item_width/2;
        result << x;
    }
    */

}

}

#endif // THUMBSSPREAD_H
