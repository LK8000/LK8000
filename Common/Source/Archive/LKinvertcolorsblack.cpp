
  bool invertcolors=false;
  if (INVERTCOLORS) {
        // In mapwindow we should have set black background under the same situation, already
        if (!EnableTerrain || !DerivedDrawInfo.TerrainValid || !RasterTerrain::isTerrainLoaded())
                invertcolors=true;
  }

