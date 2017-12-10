# -*- coding: utf-8 -*-
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.checkbox import CheckBox
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
        self.kvImage1 = Image(size_hint=(1.0, 0.9))
        # Layoutを作ってadd_widgetで順次モノを置いていく(並びは置いた順)
        kvBoxLayout1 = BoxLayout(orientation='vertical')
        kvBoxLayout1.add_widget(self.kvImage1)
        # 複数行に何か並べる場合はGridLayoutの方が楽そう
        kvGridLayout1 = GridLayout(cols = 2, size_hint=(1.0, 0.1))
        kvCheckBox1Label = Label(text = 'Sobel', halign='right')
        self.kvCheckBox1 = CheckBox(group = 'method', active= True)
        self.kvCheckBox1.bind(active = self.on_checkbox_active)
        kvCheckBox2Label = Label(text = 'Canny', halign='right')
        self.kvCheckBox2 = CheckBox(group = 'method')
        self.kvCheckBox2.bind(active = self.on_checkbox_active)
        kvGridLayout1.add_widget(kvCheckBox1Label)
        kvGridLayout1.add_widget(self.kvCheckBox1)
        kvGridLayout1.add_widget(kvCheckBox2Label)
        kvGridLayout1.add_widget(self.kvCheckBox2)
        kvBoxLayout1.add_widget(kvGridLayout1)
        self.process()
        return kvBoxLayout1

    def on_checkbox_active(self, checkbox, value):
        self.process()

    def process(self):
        if (self.kvCheckBox1.active):
            dst = cv2.Sobel(self.src, cv2.CV_8U, 1, 0, ksize = 5)
        else:
            dst = cv2.Canny(self.src, 100, 200)
        kvImage1Texture = Texture.create(size=(dst.shape[1], dst.shape[0]), colorfmt='bgr')
        kvImage1Texture.blit_buffer(cv2.merge((dst, dst, dst)).tostring(), colorfmt='bgr', bufferfmt='ubyte')
        self.kvImage1.texture = kvImage1Texture

if __name__ == '__main__':
    CvImg().run()
