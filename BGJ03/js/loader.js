function loadLevel(state, level, additionalActors) {
    var keys = 0;

    additionalActors = additionalActors || [];
    $.getJSON(level, function(json) {
        var width = json.width;
        var height = json.height;
        state.levelWidth = width * TILESIZE;
        state.levelHeight = height * TILESIZE;

        
        for(var layerindex = 0; layerindex < json.layers.length; ++layerindex) {
            var data = json.layers[layerindex].data;

            for(var y = 0; y < height; y++) {
                for(var x = 0; x < width; x++) {
                    var tileindex = data[x+width*y];

                    if(tileindex !== 0) {
                        var tilesetindex = 0;

                        while(tilesetindex < json.tilesets.length-1 && json.tilesets[tilesetindex+1].firstgid <= tileindex) {
                            ++tilesetindex;
                        }

                        var tilesetLineWidth = json.tilesets[tilesetindex].imagewidth / TILESIZE;

                        tileindex -= json.tilesets[tilesetindex].firstgid-1;

                        var tileX = (tileindex-1) % tilesetLineWidth;
                        var tileY = Math.floor((tileindex-1) / tilesetLineWidth);

                        if(json.layers[layerindex].name != "collision") {
                            var tile =  new Tile("tile-" + x + "-" + y + "-onlayer-"+layerindex, 
                                                x, y,
                                                tileX, tileY,
                                                'levels/'+json.tilesets[tilesetindex].image,
                                                tryParseInt(json.layers[layerindex].name));
                            state.addActor(tile);
                        } else {
                            if(tileindex >= 1 && tileindex <= 5) {
                                state.addActor(new CollisionTile(nextId("collisiontile-"), x, y, tileindex));
                            } else if((tileindex >= 5 && tileindex <= 10) || tileindex == 12) {
                                var id = (tileindex == 12 ? 1 : tileindex - 5);
                                var callback = (tileindex == 12 ? 
                                        function(tile) { state.playerWins(tile) } : 
                                        function(tile) { state.playerDied(tile) } );
                                state.addActor(new TriggerTile(nextId("triggertile-"), x, y, id, callback));
                            } else if(tileindex == 11) {
                                state.resetPosition.x = (x + 0.5) * TILESIZE;
                                state.resetPosition.y = y * TILESIZE;
                            } else if(tileindex == 14) {
                                state.addActor(new Box(nextId("box-"), x, y));
                            } else if(tileindex == 13) {
                                state.addActor(new Key(nextId("key-"), x, y));
                                keys++;
                            }
                        }
                    }
                }
            }
        }

        state.keys = keys;
        state.keysLeft = keys;
        
        state.flashAlpha = 1;
        if(state.level < LEVELS_WITHOUT) state.gunMode = 0;
        else if(state.level < LEVELS_WITHOUT + LEVELS_WITHOUT_SECOND) state.gunMode = 1;
        else state.gunMode = 2;

        for(var x = 0; x < width; x++) {
            state.addActor(new CollisionTile(nextId("walls-"), x, -1, 1, true));
            state.addActor(new CollisionTile(nextId("walls-"), x, height + 1, 1, true));
        }

        for(var y = 0; y < height; y++) {
            state.addActor(new CollisionTile(nextId("walls-"), -1, y, 1, true));
            state.addActor(new CollisionTile(nextId("walls-"), width + 1, y, 1, true));
        }
        
        
        state.addActor(state.levelname);
        for(var i = 0; i < additionalActors.length; ++i) {
            state.addActor(additionalActors[i]);
        }   

        state.spawnPlayer();
        
        for(var actor in state.actors) {
            if("activatePhysics" in state.actors[actor] && typeof(state.actors[actor].activatePhysics) == "function") {
                state.actors[actor].activatePhysics();
            }
        }
        
        state.player.reset(state.resetPosition);
        // state.resetPlayer();
        
        for(var actor in state.actors) {
            if(state.actors[actor].body != null) {
                state.actors[actor].setAwake(true);
            }
        }
    });
}
