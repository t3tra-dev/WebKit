/*
    Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2012 Company 100, Inc.
    Copyright (C) 2017 Sony Interactive Entertainment Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "CoordinatedGraphicsScene.h"

#if USE(COORDINATED_GRAPHICS)
#include "CoordinatedSceneState.h"
#include <WebCore/CoordinatedPlatformLayer.h>
#include <WebCore/Damage.h>
#include <WebCore/TextureMapperLayer.h>
#include <wtf/Atomics.h>
#include <wtf/SystemTracing.h>

namespace WebKit {
using namespace WebCore;

CoordinatedGraphicsScene::CoordinatedGraphicsScene(CoordinatedGraphicsSceneClient& client, CoordinatedSceneState& sceneState)
    : m_sceneState(sceneState)
    , m_client(&client)
{
}

CoordinatedGraphicsScene::~CoordinatedGraphicsScene() = default;

void CoordinatedGraphicsScene::paintToCurrentGLContext(const TransformationMatrix& matrix, const FloatRect& clipRect, bool flipY)
{
    updateSceneState();

    TextureMapperLayer& currentRootLayer = m_sceneState->rootLayer().ensureTarget();
    if (currentRootLayer.transform() != matrix)
        currentRootLayer.setTransform(matrix);

    bool sceneHasRunningAnimations = currentRootLayer.applyAnimationsRecursively(MonotonicTime::now());

    m_textureMapper->beginPainting(flipY ? TextureMapper::FlipY::Yes : TextureMapper::FlipY::No);
    m_textureMapper->beginClip(TransformationMatrix(), FloatRoundedRect(clipRect));

    std::optional<FloatRoundedRect> rectContainingRegionThatActuallyChanged;
#if ENABLE(DAMAGE_TRACKING)
    currentRootLayer.prepareForPainting(*m_textureMapper);
    if (m_client && m_damagePropagation != Damage::Propagation::None) {
        Damage frameDamage;
        WTFBeginSignpost(this, CollectDamage);
        currentRootLayer.collectDamage(*m_textureMapper, frameDamage);
        WTFEndSignpost(this, CollectDamage);

        if (m_damagePropagation == Damage::Propagation::Unified) {
            Damage boundsDamage;
            boundsDamage.add(frameDamage.bounds());
            frameDamage = WTFMove(boundsDamage);
        }

        const auto& damageSinceLastSurfaceUse = m_client->addSurfaceDamage(!frameDamage.isInvalid() && !frameDamage.isEmpty() ? frameDamage : Damage::invalid());
        if (!damageSinceLastSurfaceUse.isInvalid() && !FloatRect(damageSinceLastSurfaceUse.bounds()).contains(clipRect))
            rectContainingRegionThatActuallyChanged = FloatRoundedRect(damageSinceLastSurfaceUse.bounds());
    }
#endif

    if (rectContainingRegionThatActuallyChanged)
        m_textureMapper->beginClip(TransformationMatrix(), *rectContainingRegionThatActuallyChanged);

    WTFBeginSignpost(this, PaintTextureMapperLayerTree);
    currentRootLayer.paint(*m_textureMapper);
    WTFEndSignpost(this, PaintTextureMapperLayerTree);

    if (rectContainingRegionThatActuallyChanged)
        m_textureMapper->endClip();

    m_fpsCounter.updateFPSAndDisplay(*m_textureMapper, clipRect.location(), matrix);

    m_textureMapper->endClip();
    m_textureMapper->endPainting();

    if (sceneHasRunningAnimations)
        updateViewport();
}

void CoordinatedGraphicsScene::updateViewport()
{
    if (m_client)
        m_client->updateViewport();
}

void CoordinatedGraphicsScene::updateSceneState()
{
    if (!m_textureMapper)
        m_textureMapper = TextureMapper::create();

    m_sceneState->rootLayer().flushCompositingState(*m_textureMapper);
    for (auto& layer : m_sceneState->committedLayers())
        layer->flushCompositingState(*m_textureMapper);
}

void CoordinatedGraphicsScene::purgeGLResources()
{
    ASSERT(!m_client);

    m_sceneState->invalidateCommittedLayers();
    m_textureMapper = nullptr;
}

void CoordinatedGraphicsScene::detach()
{
    ASSERT(RunLoop::isMain());
    m_client = nullptr;
}

} // namespace WebKit

#endif // USE(COORDINATED_GRAPHICS)
