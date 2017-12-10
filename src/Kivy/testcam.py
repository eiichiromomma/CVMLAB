# -*- coding: utf-8 -*-
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.slider import Slider
from kivy.uix.image import Image
from kivy.uix.label import Label
from kivy.graphics.texture import Texture
from kivy.clock import Clock
import cv2

class CvCamera(App):
    def build(self): #UIの構築等
        self._cap = cv2.VideoCapture(0)
        # ButtonやSlider等は基本size_hintでサイズ比率を指定(絶対値の時はNoneでsize=)
        # 日本語フォントを使いたいときはfont_nameでフォントへのパス
        kvButton1 = Button(text='ボタン', size_hint=(1.0, 0.1), font_name='/usr/local/texlive/texmf-local/fonts/truetype/cjk-gs-integrate/ipag.ttf')
        kvButton1.bind(on_press = self.buttonCallback) #bindでイベントごとにコールバック指定
        # Imageに後で画像を描く
        self.kvImage1 = Image(size_hint=(1.0, 0.7))
        # Layoutを作ってadd_widgetで順次モノを置いていく(並びは置いた順)
        kvLayout1 = BoxLayout(orientation='vertical')
        kvLayout1.add_widget(self.kvImage1)
        # ここだけ2columnでLabelとSliderを並べる
        # Verticalの中に置くhorizontalなBoxLayout
        kvLayout2 = BoxLayout(orientation='horizontal', size_hint=(1.0, 0.1))
        self.kvSlider1Label = Label(text = 'Slider', size_hint=(0.3, 1.0), halign='center')
        kvSlider1 = Slider(size_hint=(0.7, 1.0))
        kvSlider1.bind(value=self.slideCallback)
        kvLayout1.add_widget(kvLayout2)
        kvLayout2.add_widget(self.kvSlider1Label)
        kvLayout2.add_widget(kvSlider1)
        # 1columnに戻る
        kvLayout1.add_widget(kvButton1)
        #カメラ待ち
        while not self._cap.isOpened():
            pass
        # 更新スケジュールとコールバックの指定
        Clock.schedule_interval(self.update, 1.0/30.0)
        return kvLayout1

    def slideCallback(self, instance, value):
        # Slider横のLabelをSliderの値に
        self.kvSlider1Label.text = 'Slider %s' % int(value)

    def buttonCallback(self, instance):
        # 何かのフラグに使える
        print('Button <%s> is pressed.' % (instance))

    def update(self, dt):
        # 基本的にここでOpenCV周りの処理を行なってtextureを更新する
        ret, frame = self._cap.read()
        frame = cv2.flip(frame, 0)
        kvTexture1 = Texture.create(size=(frame.shape[1], frame.shape[0]), colorfmt='bgr')
        kvTexture1.blit_buffer(frame.tostring(), colorfmt='bgr', bufferfmt='ubyte')
        self.kvImage1.texture = kvTexture1

if __name__ == '__main__':
    CvCamera().run()
