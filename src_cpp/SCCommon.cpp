#include "internal/SCCommon.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>

namespace SL {
namespace Screen_Capture {

    void SanitizeRects(std::vector<ImageRect> &rects, const Image &img)
    {
        for (auto &r : rects) {
            if (r.right > Width(img)) {
                r.right = Width(img);
            }
            if (r.bottom > Height(img)) {
                r.bottom = Height(img);
            }
        }
    }

    template <typename Block> class BitMap {
        static_assert(std::is_unsigned<Block>::value);
        static const size_t BitsPerBlock = sizeof(Block) * 8;

      public:
        BitMap(size_t height, size_t width) : Width(width), Height(height), Blocks((width * height) / BitsPerBlock + 1, 0) {}

        bool get(size_t x, size_t y) const
        {
            const size_t index = x * Width + y;
            const size_t nblock = index / BitsPerBlock;
            const size_t nbit = index % BitsPerBlock;

            return Blocks[nblock] & (Block(1) << nbit);
        }

        void set(size_t x, size_t y)
        {
            const size_t index = x * Width + y;
            const size_t nblock = index / BitsPerBlock;
            const size_t nbit = index % BitsPerBlock;

            Blocks[nblock] |= (Block(1) << nbit);
        }

        size_t width() const { return Width; }

        size_t height() const { return Height; }

      private:
        size_t Width;
        size_t Height;
        std::vector<Block> Blocks;
    };

    static void merge(std::vector<ImageRect> &rects)
    {
        if (rects.size() <= 2) {
            return; // make sure there is at least 2
        }

        std::vector<ImageRect> outrects;
        outrects.reserve(rects.size());
        outrects.push_back(rects[0]);

        // horizontal scan
        for (size_t i = 1; i < rects.size(); i++) {
            if (outrects.back().right == rects[i].left && outrects.back().bottom == rects[i].bottom) {
                outrects.back().right = rects[i].right;
            }
            else {
                outrects.push_back(rects[i]);
            }
        }

        if (outrects.size() <= 2) {
            rects = std::move(outrects);
            return; // make sure there is at least 2
        }

        rects.clear();
        // vertical scan
        for (auto &otrect : outrects) {
            auto found = std::find_if(rects.rbegin(), rects.rend(), [=](const ImageRect &rec) {
                return rec.bottom == otrect.top && rec.left == otrect.left && rec.right == otrect.right;
            });

            if (found == rects.rend()) {
                rects.push_back(otrect);
            }
            else {
                found->bottom = otrect.bottom;
            }
        }
    }

#define maxdist 256

    static std::vector<ImageRect> GetRects(const BitMap<uint64_t> &map)
    {
        std::vector<ImageRect> rects;
        rects.reserve(map.width() * map.height());

        for (decltype(map.height()) x = 0; x < map.height(); ++x) {
            for (decltype(map.width()) y = 0; y < map.width(); ++y) {
                if (map.get(x, y)) {
                    ImageRect rect;

                    rect.top = static_cast<decltype(rect.top)>(x * maxdist);
                    rect.bottom = static_cast<decltype(rect.bottom)>((x + 1) * maxdist);

                    rect.left = static_cast<decltype(rect.left)>(y * maxdist);
                    rect.right = static_cast<decltype(rect.right)>((y + 1) * maxdist);

                    rects.push_back(rect);
                }
            }
        }

        return rects;
    }

    std::vector<ImageRect> GetDifs(const Image &oldImage, const Image &newImage)
    {
        auto old_ptr = (const int *)StartSrc(oldImage);
        auto new_ptr = (const int *)StartSrc(newImage);

        const auto width = Width(newImage);
        const auto height = Height(newImage);

        const auto width_chunks = width / maxdist;
        const auto height_chunks = height / maxdist;

        const auto line_rem = width % maxdist;
        const auto bottom_rem = height % maxdist;

        BitMap<uint64_t> changes{static_cast<size_t>(height_chunks) + 1, static_cast<size_t>(width_chunks) + 1};

        const auto compare = [&](size_t x, size_t y, size_t npixels) {
            if (!changes.get(x, y)) {
                if (memcmp(old_ptr, new_ptr, npixels * sizeof(int))) {
                    changes.set(x, y);
                }
            }

            old_ptr += npixels;
            new_ptr += npixels;
        };

        for (int x = 0; x < height_chunks; ++x) {
            for (int i = 0; i < maxdist; ++i) { // for each row in current line of chunks
                for (int y = 0; y < width_chunks; ++y) {
                    compare(x, y, maxdist);
                }
                compare(x, width_chunks, line_rem);
            }
        }

        for (int i = 0; i < bottom_rem; ++i) {
            for (int y = 0; y < width_chunks; ++y) {
                compare(height_chunks, y, maxdist);
            }

            compare(height_chunks, width_chunks, line_rem);
        }

        auto rects = GetRects(changes);
        merge(rects);
        SanitizeRects(rects, newImage);
        return rects;
    }

    Monitor CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string &n, float scaling)
    {
        Monitor ret = {};
        ret.Index = index;

        ret.Id = id;
        assert(n.size() + 1 < sizeof(ret.Name));
        memcpy(ret.Name, n.c_str(), n.size() + 1);
        ret.OriginalOffsetX = ret.OffsetX = ox;
        ret.OriginalOffsetY = ret.OffsetY = oy;
        ret.OriginalWidth = ret.Width = w;
        ret.OriginalHeight = ret.Height = h;
        ret.Scaling = scaling;
        return ret;
    }

    Monitor CreateMonitor(int index, int id, int adapter, int h, int w, int ox, int oy, const std::string &n, float scaling)
    {
        Monitor ret = CreateMonitor(index, id, h, w, ox, oy, n, scaling);
        ret.Adapter = adapter;
        return ret;
    }

    Image CreateImage(const ImageRect &imgrect, int rowpadding, const ImageBGRA *data)
    {
        Image ret;
        ret.Bounds = imgrect;
        ret.Data = data;
        ret.BytesToNextRow = rowpadding;
        return ret;
    }
    int Index(const Monitor &mointor) { return mointor.Index; }
    int Id(const Monitor &mointor) { return mointor.Id; }
    int Adapter(const Monitor &mointor) { return mointor.Adapter; }
    int OffsetX(const Monitor &mointor) { return mointor.OffsetX; }
    int OffsetY(const Monitor &mointor) { return mointor.OffsetY; }
    void OffsetX(Monitor &mointor, int x) { mointor.OffsetX = x; }
    void OffsetY(Monitor &mointor, int y) { mointor.OffsetY = y; }
    int OffsetX(const Window &mointor) { return mointor.Position.x; }
    int OffsetY(const Window &mointor) { return mointor.Position.y; }
    void OffsetX(Window &mointor, int x) { mointor.Position.x = x; }
    void OffsetY(Window &mointor, int y) { mointor.Position.y = y; }
    int OffsetX(const Image &img) { return img.Bounds.left; }
    int OffsetY(const Image &img) { return img.Bounds.top; }
    const char *Name(const Monitor &mointor) { return mointor.Name; }
    const char *Name(const Window &mointor) { return mointor.Name; }
    int Height(const Monitor &mointor) { return mointor.Height; }
    int Width(const Monitor &mointor) { return mointor.Width; }
    void Height(Monitor &mointor, int h) { mointor.Height = h; }
    void Width(Monitor &mointor, int w) { mointor.Width = w; }
    int Height(const Window &mointor) { return mointor.Size.y; }
    int Width(const Window &mointor) { return mointor.Size.x; }
    void Height(Window &mointor, int h) { mointor.Size.y = h; }
    void Width(Window &mointor, int w) { mointor.Size.x = w; }
    int Height(const ImageRect &rect) { return rect.bottom - rect.top; }
    int Width(const ImageRect &rect) { return rect.right - rect.left; }
    int Height(const Image &img) { return Height(img.Bounds); }
    int Width(const Image &img) { return Width(img.Bounds); }
    int X(const Point &p) { return p.x; }
    int Y(const Point &p) { return p.y; }
    const ImageRect &Rect(const Image &img) { return img.Bounds; }
    const ImageBGRA *GotoNextRow(const Image &img, const ImageBGRA *current)
    {
        auto c = reinterpret_cast<const unsigned char *>(current);
        return reinterpret_cast<const ImageBGRA *>(c + img.BytesToNextRow);
    }
    bool isDataContiguous(const Image &img) { return img.isContiguous; }
    // number of bytes per row, NOT including the Rowpadding
    int RowStride(const Image &img) { return sizeof(ImageBGRA) * Width(img); }
    const ImageBGRA *StartSrc(const Image &img) { return img.Data; }
} // namespace Screen_Capture
} // namespace SL
