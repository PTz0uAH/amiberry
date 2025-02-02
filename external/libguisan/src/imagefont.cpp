/*      _______   __   __   __   ______   __   __   _______   __   __
 *     / _____/\ / /\ / /\ / /\ / ____/\ / /\ / /\ / ___  /\ /  |\/ /\
 *    / /\____\// / // / // / // /\___\// /_// / // /\_/ / // , |/ / /
 *   / / /__   / / // / // / // / /    / ___  / // ___  / // /| ' / /
 *  / /_// /\ / /_// / // / // /_/_   / / // / // /\_/ / // / |  / /
 * /______/ //______/ //_/ //_____/\ /_/ //_/ //_/ //_/ //_/ /|_/ /
 * \______\/ \______\/ \_\/ \_____\/ \_\/ \_\/ \_\/ \_\/ \_\/ \_\/
 *
 * Copyright (c) 2004, 2005, 2006, 2007 Olof Naessén and Per Larsson
 *
 *                                                         Js_./
 * Per Larsson a.k.a finalman                          _RqZ{a<^_aa
 * Olof Naessén a.k.a jansem/yakslem                _asww7!uY`>  )\a//
 *                                                 _Qhm`] _f "'c  1!5m
 * Visit: http://guichan.darkbits.org             )Qk<P ` _: :+' .'  "{[
 *                                               .)j(] .d_/ '-(  P .   S
 * License: (BSD)                                <Td/Z <fP"5(\"??"\a.  .L
 * Redistribution and use in source and          _dV>ws?a-?'      ._/L  #'
 * binary forms, with or without                 )4d[#7r, .   '     )d`)[
 * modification, are permitted provided         _Q-5'5W..j/?'   -?!\)cam'
 * that the following conditions are met:       j<<WP+k/);.        _W=j f
 * 1. Redistributions of source code must       .$%w\/]Q  . ."'  .  mj$
 *    retain the above copyright notice,        ]E.pYY(Q]>.   a     J@\
 *    this list of conditions and the           j(]1u<sE"L,. .   ./^ ]{a
 *    following disclaimer.                     4'_uomm\.  )L);-4     (3=
 * 2. Redistributions in binary form must        )_]X{Z('a_"a7'<a"a,  ]"[
 *    reproduce the above copyright notice,       #}<]m7`Za??4,P-"'7. ).m
 *    this list of conditions and the            ]d2e)Q(<Q(  ?94   b-  LQ/
 *    following disclaimer in the                <B!</]C)d_, '(<' .f. =C+m
 *    documentation and/or other materials      .Z!=J ]e []('-4f _ ) -.)m]'
 *    provided with the distribution.          .w[5]' _[ /.)_-"+?   _/ <W"
 * 3. Neither the name of Guichan nor the      :$we` _! + _/ .        j?
 *    names of its contributors may be used     =3)= _f  (_yQmWW$#(    "
 *    to endorse or promote products derived     -   W,  sQQQQmZQ#Wwa]..
 *    from this software without specific        (js, \[QQW$QWW#?!V"".
 *    prior written permission.                    ]y:.<\..          .
 *                                                 -]n w/ '         [.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT       )/ )/           !
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY         <  (; sac    ,    '
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING,               ]^ .-  %
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF            c <   r
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR            aga<  <La
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          5%  )P'-3L
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR        _bQf` y`..)a
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          ,J?4P'.P"_(\?d'.,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES               _Pa,)!f/<[]/  ?"
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT      _2-..:. .r+_,.. .
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     ?a.<%"'  " -'.a_ _,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION)                     ^
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * For comments regarding functions please see the header file.
 */

#include "guisan/imagefont.hpp"

#include <sstream>

#include "guisan/color.hpp"
#include "guisan/exception.hpp"
#include "guisan/graphics.hpp"
#include "guisan/image.hpp"

namespace gcn
{

    namespace
    {
        int computeGlyphHeight(Image* imagePtr)
        {
            if (imagePtr == nullptr)
            {
                throw GCN_EXCEPTION("Font image is nullptr");
            }
            Image& image = *imagePtr;
            const Color separator = image.getPixel(0, 0);

            for (int i = 1; i != image.getWidth(); ++i)
            {
                if (separator == image.getPixel(i, 0))
                {
                    continue;
                }
                for (int j = 1; j != image.getHeight(); ++j)
                {
                    if (separator == image.getPixel(i, j))
                    {
                        return j;
                    }
                }
                return image.getHeight();
            }
            throw GCN_EXCEPTION("Corrupt image.");
        }
    } // namespace

    ImageFont::ImageFont(const std::string& filename, const std::string& glyphs) :
        ImageFont(
            std::shared_ptr<Image>{Image::load(filename, false), std::default_delete<Image>{}},
            glyphs)
    {}

    ImageFont::ImageFont(Image* image, const std::string& glyphs) :
        ImageFont(std::shared_ptr<Image>{image, std::default_delete<Image>{}}, glyphs)
    {}

    ImageFont::ImageFont(std::shared_ptr<Image> image, const std::string& glyphs) :
        mImage(std::move(image)),
        mHeight(computeGlyphHeight(mImage.get()))
    {
        const Color separator = mImage->getPixel(0, 0);
        int x = 0;
        int y = 0;
        for (unsigned char c : glyphs)
        {
            mGlyph[c] = scanForGlyph(c, x, y, separator);
            // Update x and y with new coordinates.
            x = mGlyph[c].x + mGlyph[c].width;
            y = mGlyph[c].y;
        }

        mImage->convertToDisplayFormat();
    }

    ImageFont::ImageFont(const std::string& filename,
                         unsigned char glyphsFrom,
                         unsigned char glyphsTo) :
        mImage{Image::load(filename, false), std::default_delete<Image>{}},
        mHeight(computeGlyphHeight(mImage.get()))
    {
        const Color separator = mImage->getPixel(0, 0);

        int x = 0;
        int y = 0;
        for (int i = glyphsFrom; i < glyphsTo + 1; i++)
        {
            mGlyph[i] = scanForGlyph(i, x, y, separator);
            // Update x and y with new coordinates.
            x = mGlyph[i].x + mGlyph[i].width;
            y = mGlyph[i].y;
        }
        mImage->convertToDisplayFormat();
    }

    int ImageFont::getWidth(unsigned char glyph) const
    {
        if (mGlyph[glyph].width == 0)
        {
            return mGlyph[(int)(' ')].width + mGlyphSpacing;
        }

        return mGlyph[glyph].width + mGlyphSpacing;
    }

    int ImageFont::getHeight() const
    {
        return mHeight + mRowSpacing;
    }

    int ImageFont::drawGlyph(Graphics* graphics, unsigned char glyph, int x, int y)
    {
        // This is needed for drawing the glyph in the middle if we have spacing
        int yoffset = getRowSpacing() / 2;

        if (mGlyph[glyph].width == 0)
        {
            graphics->drawRectangle(Rectangle(
                x, y + 1 + yoffset, mGlyph[(int) (' ')].width - 1, mGlyph[(int) (' ')].height - 2));

            return mGlyph[(int)(' ')].width + mGlyphSpacing;
        }

        graphics->drawImage(mImage.get(),
                            mGlyph[glyph].x,
                            mGlyph[glyph].y,
                            x,
                            y + yoffset,
                            mGlyph[glyph].width,
                            mGlyph[glyph].height);

        return mGlyph[glyph].width + mGlyphSpacing;
    }

    void ImageFont::drawString(Graphics* graphics, const std::string& text, int x, int y, bool enabled)
    {
        unsigned int i;

        for (i = 0; i < text.size(); ++i)
        {
            drawGlyph(graphics, text.at(i), x, y);
            x += getWidth(text.at(i));
        }
    }

    void ImageFont::setRowSpacing(int spacing)
    {
        mRowSpacing = spacing;
    }

    int ImageFont::getRowSpacing()
    {
        return mRowSpacing;
    }

    void ImageFont::setGlyphSpacing(int spacing)
    {
        mGlyphSpacing = spacing;
    }

    int ImageFont::getGlyphSpacing()
    {
        return mGlyphSpacing;
    }

    Rectangle ImageFont::scanForGlyph(unsigned char glyph, int x, int y, const Color& separator) const
    {
        Color color;
        do
        {
            ++x;

            if (x >= mImage->getWidth())
            {
                y += mHeight + 1;
                x = 0;

                if (y >= mImage->getHeight())
                {
                    std::string str;
                    std::ostringstream os(str);
                    os << "Image with font is corrupt near character '";
                    os << glyph;
                    os << "'";
                    throw GCN_EXCEPTION(os.str());
                }
            }

            color = mImage->getPixel(x, y);

        } while (color == separator);

        int width = 0;

        do
        {
            ++width;

            if (x + width >= mImage->getWidth())
            {
                std::string str;
                std::ostringstream os(str);
                os << "Image with font is corrupt near character '";
                os << glyph;
                os << "'";
                throw GCN_EXCEPTION(os.str());
            }

            color = mImage->getPixel(x + width, y);

        } while (color != separator);

        return Rectangle(x, y, width, mHeight);
    }

    int ImageFont::getWidth(const std::string& text) const
    {
        int size = 0;

        for (unsigned int i = 0; i < text.size(); ++i)
        {
            size += getWidth(text.at(i));
        }

        return size - mGlyphSpacing;
    }

    int ImageFont::getStringIndexAt(const std::string& text, int x) const
    {
        int size = 0;

        for (unsigned int i = 0; i < text.size(); ++i)
        {
            size += getWidth(text.at(i));

            if (size > x)
            {
                return i;
            }
        }

        return text.size();
    }
}
