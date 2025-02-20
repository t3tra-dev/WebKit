/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "SVGPathByteStreamSource.h"

namespace WebCore {

SVGPathByteStreamSource::SVGPathByteStreamSource(const SVGPathByteStream& stream)
    : m_streamCurrent(stream.bytes().span())
{
}

bool SVGPathByteStreamSource::hasMoreData() const
{
    return !m_streamCurrent.empty();
}

SVGPathSegType SVGPathByteStreamSource::nextCommand(SVGPathSegType)
{
    return readSVGSegmentType();
}

std::optional<SVGPathSegType> SVGPathByteStreamSource::parseSVGSegmentType()
{
    return readSVGSegmentType();
}

std::optional<SVGPathSource::MoveToSegment> SVGPathByteStreamSource::parseMoveToSegment(FloatPoint)
{
    MoveToSegment segment;
    segment.targetPoint = readFloatPoint();
    return segment;
}

std::optional<SVGPathSource::LineToSegment> SVGPathByteStreamSource::parseLineToSegment(FloatPoint)
{
    LineToSegment segment;
    segment.targetPoint = readFloatPoint();
    return segment;
}

std::optional<SVGPathSource::LineToHorizontalSegment> SVGPathByteStreamSource::parseLineToHorizontalSegment(FloatPoint)
{
    LineToHorizontalSegment segment;
    segment.x = readFloat();
    return segment;
}

std::optional<SVGPathSource::LineToVerticalSegment> SVGPathByteStreamSource::parseLineToVerticalSegment(FloatPoint)
{
    LineToVerticalSegment segment;
    segment.y = readFloat();
    return segment;
}

std::optional<SVGPathSource::CurveToCubicSegment> SVGPathByteStreamSource::parseCurveToCubicSegment(FloatPoint)
{
    CurveToCubicSegment segment;
    segment.point1 = readFloatPoint();
    segment.point2 = readFloatPoint();
    segment.targetPoint = readFloatPoint();
    return segment;
}

std::optional<SVGPathSource::CurveToCubicSmoothSegment> SVGPathByteStreamSource::parseCurveToCubicSmoothSegment(FloatPoint)
{
    CurveToCubicSmoothSegment segment;
    segment.point2 = readFloatPoint();
    segment.targetPoint = readFloatPoint();
    return segment;
}

std::optional<SVGPathSource::CurveToQuadraticSegment> SVGPathByteStreamSource::parseCurveToQuadraticSegment(FloatPoint)
{
    CurveToQuadraticSegment segment;
    segment.point1 = readFloatPoint();
    segment.targetPoint = readFloatPoint();
    return segment;
}

std::optional<SVGPathSource::CurveToQuadraticSmoothSegment> SVGPathByteStreamSource::parseCurveToQuadraticSmoothSegment(FloatPoint)
{
    CurveToQuadraticSmoothSegment segment;
    segment.targetPoint = readFloatPoint();
    return segment;
}

std::optional<SVGPathSource::ArcToSegment> SVGPathByteStreamSource::parseArcToSegment(FloatPoint)
{
    ArcToSegment segment;
    segment.rx = readFloat();
    segment.ry = readFloat();
    segment.angle = readFloat();
    segment.largeArc = readFlag();
    segment.sweep = readFlag();
    segment.targetPoint = readFloatPoint();
    return segment;
}

}
