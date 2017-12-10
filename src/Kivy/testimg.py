# -*- coding: utf-8 -*-
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.slider import Slider
from kivy.uix.image import Image
from kivy.uix.label import Label
from kivy.graphics.texture import Texture
import cv2
import sys

class CvImg(App):
    def build(self): #UIの構築等
        args = sys.argv
        self.src = cv2.imread(args[1], cv2.IMREAD_GRAYSCALE)
        if self.src is None:
            return -1
        self.src = cv2.flip(self.src, 0)
        # ButtonやSlider等は基本size_hintでサイズ比率を指定(絶対値の時はNoneでsize=)
        # Imageに後で画像を描く
        self.kvImage1 = Image(size_hint=(1.0, 0.7))
        # Layoutを作ってadd_widgetで順次モノを置いていく(並びは置いた順)
        kvBoxLayout1 = BoxLayout(orientation='vertical')
        kvBoxLayout1.add_widget(self.kvImage1)
        # ここだけ2columnでkvBoxLayout2にLabelとSliderを並べてkvBoxLayout1に渡す
        # Verticalの中に置くhorizontalなBoxLayout (ここだけ2column)
        kvBoxLayout2 = BoxLayout(orientation='horizontal', size_hint=(1.0, 0.1))
        self.kvSliderLabel = Label(text = 'Threshold', size_hint=(0.3, 1.0), halign='center')
        kvSlider = Slider(size_hint=(0.7, 1.0), min=0, max=255, value=128)
        kvSlider.bind(value=self.sliderCallback)
        kvBoxLayout2.add_widget(self.kvSliderLabel)
        kvBoxLayout2.add_widget(kvSlider)
        kvBoxLayout1.add_widget(kvBoxLayout2)
        self.process(kvSlider.value)
        return kvBoxLayout1

    def sliderCallback(self, instance, value):
        self.process(value)

    def process(self, value):
        # Slider横のLabelをSliderの値に
        self.kvSliderLabel.text = 'Threshold %s' % int(value)
        ret, dst = cv2.threshold(self.src, value, 255, cv2.THRESH_BINARY)
        kvImage1Texture = Texture.create(size=(dst.shape[1], dst.shape[0]), colorfmt='bgr')
        kvImage1Texture.blit_buffer(cv2.merge((dst, dst, dst)).tostring(), colorfmt='bgr', bufferfmt='ubyte')
        self.kvImage1.texture = kvImage1Texture

if __name__ == '__main__':
    CvImg().run()
