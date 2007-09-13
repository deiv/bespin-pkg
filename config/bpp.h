/***************************************************************************
 *   Copyright (C) 2007 by Thomas L�bking                                  *
 *   thomas@bespin-icons.org                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _OPP_H_
#define _OPP_H_

#include <QApplication>
#include <X11/Xlib.h>
#include <X11/extensions/render.h>
#include <X11/Xatom.h>
#include "../fixx11h.h"

class BespinPP : public QApplication
{
    Q_OBJECT
public:
   BespinPP(int & argc, char ** argv);
   static void exportPicture(Picture pic, uint offset, uint rgb);
   static void cleanup(int sig);
private:
   void createPixmap();
private:
   QPixmap *bgPix;
   uint offset;
};

#endif // _OPP_H_