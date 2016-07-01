#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    group.setName("FazerParticles");
    parameters.setName("offsetControl");
    ofParameterGroup aligning;
    aligning.add(scale1.set("scale1",0,3,0));
    
    ofParameterGroup one,two,three,four;
    one.setName("one");
    two.setName("two");
    three.setName("three");
    four.setName("four");
    
    one.add(offSet1X.set("offSet1X",0,-100,900));
    one.add(offSet1Y.set("offSet1Y",0,-500,500));
    
    // this overlap is always 0.
    two.add(offSet2X.set("offSet2X",0,700,1700));
    two.add(offSet2Y.set("offSet2Y",0,-500,500));
   // two.add(scale2.set("scale2",0,3,0));
    
    two.add(overLap1.set("overLap1",0,-700,700));
   
    three.add(offSet3X.set("offSet3X",0,1500,2500));
    three.add(offSet3Y.set("offSet3Y",0,-500,500));
  //  three.add(scale3.set("scale3",0,3,0));
    
    three.add(overLap2.set("overLap2",0,-700,700));
    
    four.add(offSet4X.set("offSet4X",0,2300,3300));
    four.add(offSet4Y.set("offSet4Y",0,-500,500));
   // four.add(scale4.set("scale4",0,3,0));
    
    four.add(overLap3.set("overLap3",0,-700,700));
    aligning.add(numAttractionP.set("numAttraction",5,1,20));
    
    aligning.add(one);
    aligning.add(two);
    aligning.add(three);
    aligning.add(four);
    
    
    
    visualControl.setName("visuals");
    visualControl.add(swarm.set("swarm", true));
    visualControl.add(cluster.set("clusters", true));
    visualControl.add(cAttraction.set("cAttraction",0,0,1));
    visualControl.add(pRepulsion.set("pRepulsion",0,0,1));
    visualControl.add(blend_ADD.set("blend_ADD", true));
    visualControl.add(oldApp.set("evenSizes", true));
    visualControl.add(fluidcolor.set("all same color", true));
    visualControl.add(blurImg.set("blurImg", true));
    visualControl.add(evenColor.set("uniform Color cluster", true));
    visualControl.add(sNear.set("sNear",0,1,200));
    visualControl.add(sFar.set("sFar",0,1,200));
    visualControl.add(b2dRepulsion.set("b2dRepulsion",0,1,200));

    
    visualControl.add(density.set("density",0,0,2));
    visualControl.add(bounce.set("bounce",0,0,2));
    visualControl.add(friction.set("friction",0,0,2));
  
    
    ofParameterGroup colorControl;
    colorControl.add(clusterRange1.set("background",ofColor(255,255), ofColor(0,0),ofColor(255)));
    colorControl.add(swarmColor.set("swarmnBox2dColor",ofColor(255,255), ofColor(0,0),ofColor(255)));
    colorControl.add(clusterRange2.set("cluster color",ofColor(255,255), ofColor(0,0),ofColor(255)));
    
    group.add(aligning);
    group.add(parameters);
    group.add(visualControl);
    group.add(colorControl);

    gui.setup(group);
    gui.loadFromFile("settings.xml");
    
    
    // osc receiver setup:
    for(int i = 0; i<4;i++){
        ofxOscReceiver rec = *new ofxOscReceiver;
        int port = 8001+i;
        rec.setup(port);
        receivers.push_back(rec);
    }
    
    //15 blob limit for each kinect:
    blobs.resize(4);
    for(int i = 0; i< blobs.size();i++){
        blobs[i].resize(15);
    }

    
    pointSplineFbo.allocate(RES_W,RES_H);
    pointSplineFbo.begin();
    ofClear(0);
    pointSplineFbo.end();
    
    finalRender.allocate(RES_W,RES_H);
    finalRender.begin();
    ofClear(0);
    finalRender.end();
    

    sparkImg.allocate(128,128, GL_RGBA);
    ofDisableArbTex();
     ofLoadImage(sparkImg, "dot.png");
    ofEnableArbTex();
    
    solid.allocate(128,128, GL_RGBA);
    ofDisableArbTex();
    ofLoadImage(solid, "clear.png");
    ofEnableArbTex();
    
    pointSpline.load("shader/shader");
 
    if(useB2d){
    
        //box2d setup
        box2d.init();
        box2d.setGravity(0, 0);
        box2d.createGround(0, RES_H, RES_W, RES_H);
        box2d.setFPS(30.0);
        box2d.registerGrabbing();
    }
    ofImage fCol;
    fCol.load("fazerColors.png");
    int stripeW = fCol.getWidth() / 5;
    for(int i = stripeW/2; i< fCol.getWidth();i+=stripeW){
        ofColor c = fCol.getColor(i,stripeW);
        fazerColors.push_back(c);
        
    }
    cout << fazerColors.size()<<endl;
    // swarms
    if(!useB2d){
        int num = SWARM_NUM;
        ofMesh colorMesh;
        int cIndx = 0;
        swarmParticles.assign(num, swarmParticle());
        for(unsigned int i = 0; i < swarmParticles.size(); i++){
           // swarmParticles[i].setAttractPoints(&attractPoints);
            //        swarmParticles[i].sNear = &sNear;
            //        swarmParticles[i].sFar = &sFar;
            swarmParticles[i].reset();
            
            pointSizes.push_back(ofRandom(5, 35));
            swarmParticles[i].radius =pointSizes.back();
            colorMesh.addVertex(ofVec3f(fazerColors[cIndx].r,fazerColors[cIndx].g,fazerColors[cIndx].b));
            cIndx = i%fazerColors.size();
            
        }
        vbo.setNormalData(&colorMesh.getVertices()[0], (int)SWARM_NUM, GL_STATIC_DRAW);
        
    }else{
        int num = SWARM_NUM;
        ofMesh colorMesh;
        int cIndx = 0;
       // swarmParticles.assign(num, swarmParticle());
        int fours = 0;
        
        for(unsigned int i = 0; i < num; i++){
            fours = (i*4) / num;
            
            
            customParticles.push_back(shared_ptr<CustomParticle>(new CustomParticle));
            CustomParticle * p = customParticles.back().get();
            float r = ofRandom(5, 35);		// a random radius 4px - 20px
            //float density, float bounce, float friction
            p->setPhysics(ofRandom(0.1,10), ofRandom(0.1,1.2) ,ofRandom(0.1,3));
            p->setup(box2d.getWorld(), ofRandom((fours-1)*RES_W/4,fours*RES_W/4), ofRandom(100,RES_H-100), r);
            p->radius=p->getRadius();
            p->setVelocity(ofRandom(-0.5,0.5), ofRandom(-0.5,0.5));
            p->attractionPoint = ofVec2f(fours*RES_W/4 +RES_W/8, RES_H/2);
            p->num =fours;
            pointSizes.push_back(r);
            
            colorMesh.addVertex(ofVec3f(fazerColors[cIndx].r,fazerColors[cIndx].g,fazerColors[cIndx].b));
            cIndx = i%fazerColors.size();
            
        }
        vbo.setNormalData(&colorMesh.getVertices()[0], (int)SWARM_NUM, GL_STATIC_DRAW);
    
    }
    
//    post.init(RES_W,RES_H);
//    
//    post.createPass<BleachBypassPass>()->setEnabled(false);
//    post.createPass<BloomPass>()->setEnabled(false);
//    post.createPass<ToonPass>()->setEnabled(false);
//    post.createPass<RGBShiftPass>()->setEnabled(false);
//    post.createPass<NoiseWarpPass>()->setEnabled(false);
//    post.createPass<ContrastPass>()->setEnabled(false);
//    post.createPass<LimbDarkeningPass>()->setEnabled(false);
//    post.createPass<RimHighlightingPass>()->setEnabled(false);
//    post.createPass<RGBShiftPass>()->setEnabled(false);
    
    ofMesh colorMesh;
    int cIndx = 0;
    //cluster / binned sys
    for(int i = 0 ; i<CLUSTER_NUM; i++){
        int size =ofRandom(20,100);
        int num = ofRandom(50,80);
        binnedSystem b;
        clusters.push_back(b);
        clusters.back().setup(ofVec2f((RES_W/(CLUSTER_NUM+1))*(i+1),ofRandom(100, RES_H-100)),num, size);
        //clusters.back().attractPoints = &attractPoints;
        //clusters.back().tree = &theTrees[int(ofRandom(theTrees.size()))];
        for(int u = 0; u< num;u++){
            colorMesh.addVertex(ofVec3f(fazerColors[cIndx].r,fazerColors[cIndx].g,fazerColors[cIndx].b));
            cIndx = u%fazerColors.size();
            clusters.back().pSizes.push_back(ofRandom(20,size));
            
        }
        clusters.back().vbo.setNormalData(&colorMesh.getVertices()[0], (int)num, GL_STATIC_DRAW);
    }
    string path = "animals";
    ofDirectory dir(path);

    dir.allowExt("mov");
    dir.listDir();
    
    for(int i = 0; i<dir.size();i+=2){
        Animal ani = *new Animal;
        ani.setup(ofVec2f(i*200+200,RES_H/2),"/ani/"+ofToString(int(i/2),0),dir.getPath(i),dir.getPath(i+1),&attractPoints);
        animals.push_back(ani);
    }
    
   
 //   soundGrid.resize(13);
 //   soundToggle.resize(13);
    
    int stepX = RES_W/13;
    int stepY = RES_H/4;
    for(int i = 0 ; i< 13 ; i++){
        int mod = 0;
        for(int u = 0; u<3;u++){
            soundParticle s = *new soundParticle ;
            mod = i%2;
            s.pos =ofVec2f(i*stepX + stepX/2, u*stepY+mod*100+100);
            s.toggle = false;
            s.name = "/"+ofToString(i)+"/"+ofToString(u);
            sounds.push_back(s);
//            bool b = false;
//            soundGrid[i].push_back(ofVec2f(i*stepX + stepX/2, u*stepY+mod*100+100));
//            soundToggle[i].push_back(b);
        }
    }
    soundSender.setup("localhost",3000);
    drawGui = true;
    bDebug = true;
    attractPoints.resize(4);
    syphon.setName("FazerParticles");
}

//--------------------------------------------------------------
void ofApp::update(){


    vector<float>ofsetlistX;
    ofsetlistX={offSet1X,offSet2X,offSet3X,offSet4X};
    
    vector<float>ofsetlistY;
    ofsetlistY={offSet1Y,offSet2Y,offSet3Y,offSet4Y};
    
    vector<float>scaleList;
    scaleList={scale1,scale1,scale1,scale1};


    for(int r = 0; r<receivers.size();r++){
        while(receivers[r].hasWaitingMessages()){
            
            
            ofxOscMessage msg;
            receivers[r].getNextMessage(msg);
            
            
            
            if(msg.getAddress()=="/numPeople"){
                for(int i=msg.getArgAsInt(0);i<blobs[r].size();i++){
                    blobs[r][i].clear();
                    
                }
            }
            
            if(msg.getNumArgs()>3){
                int pointCloudIndx=0;
                pointCloudIndx = ofToInt(ofSplitString(msg.getAddress(),"/")[1]);
                if(pointCloudIndx<blobs[r].size()){
                    blobs[r][pointCloudIndx].clear();
    
                    for(int  i = 0 ; i< msg.getNumArgs();i+=2){
                        
                        ofVec2f pt;
                        pt.x= msg.getArgAsFloat(i)*scaleList[r] + ofsetlistX[r];
                        pt.y= msg.getArgAsFloat(i+1)*scaleList[r] + ofsetlistY[r];;
                        blobs[r][pointCloudIndx].addVertex(pt);
                        
                    }
                    if(blobs[r][pointCloudIndx].size()>0) blobs[r][pointCloudIndx].addVertex(blobs[r][pointCloudIndx].getVertices()[0]);

                }
            }
        }
    }
    
    
   // add random blob
    if(soudoLine){
        blobs[0][0].clear();
        blobs[0][1].clear();
        //testPoly
        ofPolyline line;
        float i = 0;
        while (i < TWO_PI ) { // make a heart
            float r = (2-2*sin(i) + sin(i)*sqrt(abs(cos(i))) / (sin(i)+1.4)) * -80;
            float x = ofGetWidth()/2 + cos(i) * r;
            float y = ofGetHeight()/2 + sin(i) * r;
            line.addVertex(ofVec2f(x+RES_W/6+offSet1X,y+offSet1Y));
            i+=0.005*HALF_PI*0.5;
        }
        
        line.close(); // close the shape
        
        ofPolyline line2;
        i = 0;
        while (i < TWO_PI ) { // make a heart
            float r = (2-2*sin(i) + sin(i)*sqrt(abs(cos(i))) / (sin(i)+1.4)) * -80;
            float x = ofGetWidth()/2 + cos(i) * r ;
            float y = ofGetHeight()/2 + sin(i) * r;
            line2.addVertex(ofVec2f(x+offSet1X,y+offSet1Y));
            i+=0.005*HALF_PI*0.5;
        }
        
        line2.close(); // close the shape
        blobs[0][0]=line;
        blobs[0][1]=line2;
    }


    for(int i = 0; i<attractPoints.size();i++ )attractPoints[i].clear();
    
    centroids.clear();
    
    vector<int>overLaps;
    overLaps={0,overLap1,overLap2,overLap3};
    int numBlobs=0;
    for(int i = 0; i<blobs.size();i++){
        for(int u = 0; u<blobs[i].size();u++){
            
            ofPolyline p=blobs[i][u];
            numBlobs++;
            if(p.size()>0){
                centroids.push_back(p.getCentroid2D());
                attractPoints[i].push_back(p.getCentroid2D());
                for( int pt = 0; pt < p.getVertices().size(); pt+=numAttractionP) {
                    int x = p.getVertices().at(pt).x;
                    int aNum = int(ofMap(x, 0, RES_W, 0, 4));
                   
                    if(x>overLaps[i]+ofsetlistX[i])attractPoints[aNum].push_back(p.getVertices().at(pt));
                    
                }
            }
        }
    }
    

    if(bDebug)attractPoints[1].push_back(ofPoint((RES_W/ofGetWidth() ) * mouseX, (RES_W/ofGetWidth())*mouseY)); // add mouse interaction on debug

    

    for(int i = 0; i<sounds.size();i++){
      //  for(int u = 0; u<soundGrid[i].size();u++){
            

        bool on = false;
        int iter = 0;
        
        for(int a = 0; a<blobs.size();a++){
            for(int b = 0; b<blobs[a].size();b++){
                
                ofPolyline p=blobs[a][b];
                if(p.size()>0){
                    
                    if(abs(centroids[iter].x - sounds[i].pos.x)<RES_W/6){
                        
                        if( insidePolygon(sounds[i].pos, p)){
                            on=true;
                        }
                    }
                    iter ++;
                }
            }
        }
        
        if(on && !sounds[i].toggle){
            ofxOscMessage m;
            m.setAddress(sounds[i].name);
            m.addInt32Arg(1);
            soundSender.sendMessage(m);
            sounds[i].toggle=true;
        }
        if(!on && sounds[i].toggle){
            sounds[i].toggle = false;
        }
    }
    
    for(int i = 0 ; i< animals.size();i++){
        animals[i].update();
        if(animals[i].sendOsc){
            ofxOscMessage m;
            m.setAddress(animals[i].oscAddress);
            m.addIntArg(1);
            soundSender.sendMessage(m);
        }
        for(int u = i+1; u<animals.size();u++){
            if(abs(animals[u].pos.x-animals[i].pos.x )<140){
                animals[u].vel.x *=-1;
                animals[i].vel.x *=-1;
            }
        }
    }
    
    if(swarm){
        if(useB2d){
            
            // clusters
            vector<ofVec2f>repelFrom;
            bool bRepelFrom = false;

            for(int i = 0; i<customParticles.size();i++){

                int cNum =customParticles[i]->num;
                
                int x1 = customParticles[i]->getPosition().x;
                int y1 = customParticles[i]->getPosition().y;
                
                // repel from animals, version shit
                for(int u = 0 ; u<animals.size();u++){
                    if(!animals[u].touched){
                        int x2 = animals[u].pos.x ;
                        int y2 = animals[u].pos.y ;
                        
                        if(1/b2InvSqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))<150){
                            customParticles[i]->addRepulsionForce(animals[u].pos ,b2dRepulsion);
                            //customParticles[i]->setPosition(ofRandom(customParticles[i]->num*(RES_W/4), (customParticles[i]->num+1)*(RES_W/4)), -100);
                        }
                    }
                }
                
                for(int a= 0;a<attractPoints[cNum].size();a++){
                    
                    
                    
                    
                    int x2 = attractPoints[cNum][a].x;
                    int y2 = attractPoints[cNum][a].y;
                    
                    if(1/b2InvSqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))<sNear){
                        customParticles[i]->addRepulsionForce(attractPoints[cNum][a],b2dRepulsion);
                        // customParticles[i]->setPosition(ofRandom(customParticles[i]->num*(RES_W/4), (customParticles[i]->num+1)*(RES_W/4)), -100);
                        
                    }
                }
                

                
              //  for(int u = 0; u<centroids.size();u++) customParticles[i]->addRepulsionForce(centroids[u],b2dRepulsion);
            }
//            polyShapes.clear();
//           // rects.clear();
//            for(int i = 0; i<blobs.size();i++){
//                for(int u = 0; u<blobs[i].size();u++){
//                    
//                    ofPolyline drawing = blobs[i][u];
//                    if(drawing.size()>3 && drawing.getBoundingBox().getArea()>15){
//                        
//                        drawing.setClosed(true);
//                        // save the outline of the shape
//                        
//                        shared_ptr<ofxBox2dPolygon> poly = shared_ptr<ofxBox2dPolygon>(new ofxBox2dPolygon);
////                        shared_ptr<ofxBox2dRect> rect = shared_ptr<ofxBox2dRect>(new ofxBox2dRect);
////                        rect->setPhysics(density,bounce,friction);
////                        rect->setup(box2d.getWorld(), drawing.getBoundingBox());
////                        rects.push_back(rect);
//                        poly->addVertices(drawing.getVertices());
//                        poly->setPhysics(density, bounce, friction);
//                        poly->triangulatePoly();
//                        poly.get()->create(box2d.getWorld());
//                        polyShapes.push_back(poly);
//                        
//                    }
//                }
//            }
            box2d.update();
            
            mesh.clear();
            for(unsigned int i = 0; i < customParticles.size(); i++){
                customParticles[i]->update();
//                for(int i = 0 ; i<centroids.size();i++){
//                    customParticles[i]->addRepulsionForce(centroids[i],b2dRepulsion);
//                }

                mesh.addVertex(ofVec3f(float(customParticles[i]->getPosition().x),float(customParticles[i]->getPosition().y),pointSizes[i]));
            }
            // load to vbo for pointspline replacement
            vbo.setVertexData(&mesh.getVertices()[0], SWARM_NUM, GL_STATIC_DRAW);
        }
        
//        else{
//            mesh.clear();
//            for(unsigned int i = 0; i < swarmParticles.size(); i++){
//                swarmParticles[i].sNear = sNear;
//                swarmParticles[i].sFar = sFar;
//                swarmParticles[i].update();
//                //pointSizes[i] += sin(ofGetElapsedTimef() *10) -5;
//                mesh.addVertex(ofVec3f(swarmParticles[i].pos.x,swarmParticles[i].pos.y,pointSizes[i] ));
//            }
//            vbo.setVertexData(&mesh.getVertices()[0], SWARM_NUM, GL_STATIC_DRAW);
//        }
    }
//    int red = ((sin(ofGetElapsedTimef()/10))+1) * 160/2;
//    clusterRange2.set(ofColor(red,clusterRange2->g,255-red));
    if(cluster){
        //int red = int(ofMap(numBlobs,0,5,0,160));
        
        
        for(int i = 0 ; i< clusters.size();i++){
            clusters[i].update();
            clusters[i].oldApp = oldApp;
            if(numBlobs==0)  clusters[i].centerAttraction = 0;
            else clusters[i].centerAttraction = cAttraction;
         //   clusters[i].particleRepulsion = pRepulsion;
        }
    }
   
    //pointspline replacement
    if(cluster||swarm||bBox2d){
        pointSplineFbo.begin();
        ofClear(0, 0);
        glDepthMask(GL_FALSE);
        ofEnablePointSprites();
        ofEnableAlphaBlending();
        if(blend_ADD)ofEnableBlendMode(OF_BLENDMODE_ADD);
        
        pointSpline.begin();
        if(evenColor)pointSpline.setUniform1f("evenColor", 1.);
        else pointSpline.setUniform1f("evenColor", 0.);
        
        if(blurImg)pointSpline.setUniform1f("addthis", 7.5);
        else pointSpline.setUniform1f("addthis", 2.2);
        



        
        if(cluster){
            int cIndex = 0;
            for(int i = 0 ; i< clusters.size();i++){
                cIndex = i%fazerColors.size();
            
              if(fluidcolor)pointSpline.setUniform3f("col", float(clusterRange2->r+clusters[i].r), float(clusterRange2->g+clusters[i].g), float(clusterRange2->b+clusters[i].b));
                
              else pointSpline.setUniform3f("col", float(fazerColors[cIndex].r), float(fazerColors[cIndex].g), float(fazerColors[cIndex].b));

               if(!blurImg) solid.bind();
               else sparkImg.bind();
                clusters[i].vbo.draw(GL_POINTS, 0, (int)clusters[i].particleSystem.size());
                if(!blurImg) solid.unbind();
                else sparkImg.unbind();
                
                
            }
        }
        
        if(swarm){
            
            int cIndex = 0;
            // bind the texture so that when all the points
            // are drawn they are replace with our dot image
            // pointSpline.setUniform3f("col", float(swarmColor->r), float(swarmColor->g), float(swarmColor->b));
            // if(!evenColor)pointSpline.setUniform3f("col", float(fazerColors[cIndex].r), float(fazerColors[cIndex].g), float(fazerColors[cIndex].b));
            
            if(!fluidcolor)pointSpline.setUniform3f("col", float(swarmColor->r), float(swarmColor->g), float(swarmColor->b));
            if(fluidcolor)pointSpline.setUniform3f("col", float(clusterRange2->r), float(clusterRange2->g), float(clusterRange2->b));
            
            if(!blurImg) solid.bind();
            else sparkImg.bind();
            vbo.draw(GL_POINTS, 0, (int)SWARM_NUM);
            if(!blurImg) solid.unbind();
            else sparkImg.unbind();
            
        }
        
        pointSpline.end();
        
        glDepthMask(GL_TRUE);
        
        pointSplineFbo.end();
    
        ofDisablePointSprites();
        ofDisableBlendMode();
        ofDisableAlphaBlending();
    }

    
    
    // final render
    finalRender.begin();
  //  post.begin();
    ofSetColor(255);
    ofClear(0);
    
    ofEnableAlphaBlending();

  //  ofBackgroundGradient(clusterRange1,clusterRange2, OF_GRADIENT_LINEAR);
    
    

    
    
    ofSetColor(255);
    if(cluster||swarm){
        pointSplineFbo.draw(0,0);
    }
    

   for(auto a: animals)a.draw();

    
    
    if(bDebug){
        ofSetColor(255,255,0);
        ofSetLineWidth(4);
        ofSetColor(255,255,0,200);
        ofDrawRectangle(offSet2X, 0, overLap1, RES_H);
        ofDrawRectangle(offSet3X, 0, overLap2, RES_H);
        ofDrawRectangle(offSet4X, 0, overLap3, RES_H);
        
        ofSetColor(ofColor::red);
        ofDrawLine(offSet1X, 0, offSet1X, RES_H);
        ofDrawLine(offSet2X, 0, offSet2X, RES_H);
        ofDrawLine(offSet3X, 0, offSet3X, RES_H);
        ofDrawLine(offSet4X, 0, offSet4X, RES_H);
        
        ofSetColor(ofColor::green);
        ofDrawLine(0, offSet1Y, RES_W, offSet1Y);
        ofDrawLine(0, offSet2Y, RES_W, offSet2Y);
        ofDrawLine(0, offSet3Y, RES_W, offSet3Y);
        ofDrawLine(0, offSet4Y, RES_W, offSet4Y);
        

        for(int i=0; i<attractPoints.size(); i++) {
            for(int u=0; u<attractPoints[i].size(); u++) {
                ofSetColor(i*(255/4), 255-i*(255/4), i*(255/4));
                ofDrawCircle(attractPoints[i][u],5);
            }
        }
        
        ofSetLineWidth(2);
        ofSetColor(ofColor::white);
        for(int i = 0; i<RES_W;i+=240){
            ofDrawLine(i, 0, i, RES_H);
            ofDrawBitmapStringHighlight(ofToString(i)+ " m", i+10, 10);
        
        }
        
        for(auto s:sounds){
            ofDrawCircle(s.pos,20);
        }
        ofNoFill();
        ofSetColor(255);
        ofSetLineWidth(3);
        for(int i = 0; i<animals.size();i++){
            ofDrawCircle(animals[i].pos,150);
        }

    }
  //  post.end();
    finalRender.end();

    syphon.publishTexture(&finalRender.getTexture());
    
}



//--------------------------------------------------------------
void ofApp::draw(){
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));

    if( drawGui )
    {
         gui.draw();
    }
    ofTranslate(250,10);
    ofSetColor(0, 255, 255);
    ofDrawBitmapString("Number keys toggle effects, mouse rotates scene", 10, 20);
    for (unsigned i = 0; i < post.size(); ++i)
    {
        if (post[i]->getEnabled()) ofSetColor(0, 255, 255);
        else ofSetColor(255, 0, 0);
        ostringstream oss;
        oss << i << ": " << post[i]->getName() << (post[i]->getEnabled()?" (on)":" (off)");
        ofDrawBitmapString(oss.str(), 10, 20 * (i + 2));
    }
    ofSetColor(255);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key=='d')bDebug = !bDebug;
    if(key=='s')drawGui = !drawGui;
    if(key=='l')soudoLine=!soudoLine;
    unsigned idx = key - '0';
    if (idx < post.size()) post[idx]->setEnabled(!post[idx]->getEnabled());

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


