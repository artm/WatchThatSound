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
        , items(N), gaps(N-1)
    {
        for(int i = 0; i<N; i++) {
            items[i].anchor = items[i].pos = anchors[i];
        }
        for(int i = 0; i<N-1; i++) {
                gaps[i].left = &items[i];
                gaps[i].right = &items[i+1];
                items[i].right = &gaps[i];
                items[i+1].left = &gaps[i];
        }
    }

    void apply_hard_limits()
    {
        for(int i=0; i< items.size(); i++) {
            Item& item = items[i];
            item.pos = std::max( item_width/2, std::min( item.pos, total_width-item_width/2) );
        }
    }

    int measure_gaps()
    {
        apply_hard_limits();
        int total = 0;
        // measure gaps
        for(int i = 0; i<gaps.size(); i++) {
            Gap& gap = gaps[i];
            int x1 = gap.left->pos + item_width/2;
            int x2 = gap.right->pos - item_width/2;
            total += gap.pressure = std::max( 0, x1 - x2 );
        }
        return total;
    }

    void run(double k)
    {
        int iterations = 0;

        int tot;
        while( (tot=measure_gaps()) > max_pressure  && iterations++ < 100) {
            for(int i = 0; i<items.size(); i++) {
                Item& item = items[i];
                double pressure = 0.0;
                if (item.left)
                    pressure += item.left->pressure;
                if (item.right)
                    pressure -= item.right->pressure;
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
#define USE_SPREAD

#ifdef USE_SPREAD

    if (anchors.size() < 1)
        return;

    detail::Spread spreader( total_width, item_width, anchors );
    spreader.run(0.5);
    for(int i = 0; i<spreader.items.size(); i++) {
        int x = spreader.items[i].pos - item_width/2;
        result << x;
    }
#else
    foreach( int cx, anchors ) {
        result << std::max(0, std::min( total_width - item_width, cx - item_width/2));
    }
#endif

}

}

#endif // THUMBSSPREAD_H
