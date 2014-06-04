/*
 *  Copyright (C) 2014 Ekaitz Zárraga <ekaitz.zarraga@gmail.com>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1 
import QtGStreamer 1.0
import "core"

Rectangle{
  
  id: root
  height: 450; width: 750
  color: "black"
  
  //SIGNALS to outside
     signal hangupClicked()
     signal holdClicked()
     signal soundClicked (bool toggled)
     signal showMyVideoClicked (bool toggled)
     signal showDialpadClicked (bool toggled)
     signal exitFullScreen()
  
  //SIGNALS from outside
     signal soundChangeState(bool toggled)
     signal showMyVideoChangeState(bool toggled)
     signal showDialpadChangeState(bool toggled)
     
     focus: true
     Keys.enabled: true
     Keys.onEscapePressed: {exitFullScreen()}
     
     onSoundChangeState: toolbar.soundChangeState(!toggled)//mute VS sound active (they are oposites)
     onShowMyVideoChangeState:{ showPreviewVideo(toggled); toolbar.showMyVideoChangeState(toggled)}
     onShowDialpadChangeState: toolbar.showDialpadChangeState(toggled)
     
    function changeHoldIcon(icon){
       toolbar.changeHoldIcon(icon)
    }
     
    function setLabel(name, imageUrl)
    {
       label.text= name
       label.image=imageUrl
    }
    function showVideo(show)
    {
      videoWidget.visible=show
      label.visible=!show
    }
    
    function showPreviewVideo(show)
    {
      if(show){
	 sendingVideo.visible=true
      }else{
	 sendingVideo.visible=false
      }
    }
    
    function setHoldEnabled(enable){toolbar.setHoldEnabled(enable)}
    function setSoundEnabled(enable){toolbar.setSoundEnabled(enable)}    
    function setShowDialpadEnabled(enable){toolbar.setShowDialpadEnabled(enable)} 
     
    onShowMyVideoClicked:
    {
       showPreviewVideo(toggled)
    }
     
  Rectangle{
    id: receivingVideo
      x: 70
      y: 10
      
      anchors.right: parent.right
      anchors.rightMargin: 70
      anchors.left: parent.left
      anchors.leftMargin: 70
      anchors.top: parent.top
      anchors.topMargin: 10
      anchors.bottom: parent.bottom
      anchors.bottomMargin: 20
      
    border.width: 2
    color: "transparent"
  
    Label{
      id: label
      text: "Calling"
      image: ""
      visible: true
    }
    
    VideoItem{
    id: videoWidget
    anchors.fill: parent    
    surface: videoSurface
    visible: false
    }
  }

  Rectangle{
    id: sendingVideo
    width: 200
    height: 150
    anchors.right: parent.right
    anchors.rightMargin: 20
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 70
    border.width: 2
    border.color: "dimgray"
    
    VideoItem {
      id: videoPreviewWidget
      anchors.fill: parent
      surface: videoPreviewSurface
    }
  }  
  
  Toolbar{
    id: toolbar
     width: parent.width
     anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom}
     onHangup: root.hangupClicked()
     onHold: root.holdClicked()
     onSound: root.soundClicked(!toggled) //The app uses Mute button, the oposite
     onShowMyVideo: root.showMyVideoClicked(toggled)
     onShowDialpad: root.showDialpadClicked(toggled)
  }  
}
