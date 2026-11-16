#pragma once

// Native inventory records have independent anchors in the 6 x 100 cell grid.
struct CBagLayout {
    int anchors[_MAX_INVEN_ITEM], widths[_MAX_INVEN_ITEM], heights[_MAX_INVEN_ITEM];
    int cells[600];
    CBagLayout() { Clear(); }
    void Clear() {
        for (int i=0;i<_MAX_INVEN_ITEM;++i) { anchors[i]=-1; widths[i]=heights[i]=1; }
        for (int& cell:cells) cell=-1;
    }
    void Remove(int id) {
        anchors[id]=-1;
        for (int& cell:cells) if (cell==id) cell=-1;
    }
    bool Fits(int x,int y,int width,int height) const {
        if (x<0 || y<0 || width<1 || height<1 || x+width>6 || y+height>100) return false;
        for (int r=y;r<y+height;++r)
            for (int c=x;c<x+width;++c) if (cells[r*6+c]>=0) return false;
        return true;
    }
    bool Place(int id,int width,int height,int x=-1,int y=-1) {
        if (!Fits(x,y,width,height)) {
            bool found=false;
            for (y=0;y<100 && !found;++y)
                for (x=0;x<6;++x) if (Fits(x,y,width,height)) { found=true; break; }
            if (!found) return false;
            --y;
        }
        anchors[id]=y*6+x; widths[id]=width; heights[id]=height;
        for (int r=y;r<y+height;++r)
            for (int c=x;c<x+width;++c) cells[r*6+c]=id;
        return true;
    }
};
