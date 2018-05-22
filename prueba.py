# coding: utf-8
from kivy.uix.button import Button
from kivy.base import runTouchApp
from kivy.lang import Builder
from kivy.factory import Factory as F
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout

Builder.load_string('''
<DDButton@Button>:
    size_hint_y: None
    height: '50dp'
    # Double .parent because dropdown proxies add_widget to container
    on_release: self.parent.parent.select(self.text)
''')

class FilterDD(F.DropDown):
    ignore_case = F.BooleanProperty(True)

    def __init__(self, **kwargs):
        self._needle = None
        self._order = []
        self._widgets = {}
        super(FilterDD, self).__init__(**kwargs)

    options = F.ListProperty()
    def on_options(self, instance, values):
        _order = self._order
        _widgets = self._widgets
        changed = False
        for txt in values:
            if txt not in _widgets:
                _widgets[txt] = btn = Button(text=txt, size_hint_y = None, height = '50dp')
                _order.append(txt)
                changed = True
        for txt in _order[:]:
            if txt not in values:
                _order.remove(txt)
                del _widgets[txt]
                changed = True
        if changed:
            self.apply_filter(self._needle)

    def apply_filter(self, needle):
        self._needle = needle
        self.clear_widgets()
        _widgets = self._widgets
        add_widget = self.add_widget
        ign = self.ignore_case
        _lcn = needle and needle.lower()
        for haystack in self._order:
            _lch = haystack.lower()
            if not needle or ((ign and _lcn in _lch) or 
                         (not ign and needle in haystack)):
                add_widget(_widgets[haystack])

class FilterDDTrigger(F.BoxLayout):
    def __init__(self, **kwargs):
        super(FilterDDTrigger, self).__init__(**kwargs)
        self._prev_dd = None
        self._textinput = ti = F.TextInput(multiline=False)
        ti.bind(text=self._apply_filter)
        ti.bind(on_text_validate=self._on_enter)
        self._button = btn = F.Button(text=self.text)
        btn.bind(on_release=self._on_release)
        self.add_widget(btn)

    text = F.StringProperty('Open')
    def on_text(self, instance, value):
        self._button.text = value

    dropdown = F.ObjectProperty(None, allownone=True)
    def on_dropdown(self, instance, value):
        _prev_dd = self._prev_dd
        if value is _prev_dd:
            return
        if _prev_dd:
            _prev_dd.unbind(on_dismiss=self._on_dismiss)
            _prev_dd.unbind(on_select=self._on_select)
        if value:
            value.bind(on_dismiss=self._on_dismiss)
            value.bind(on_select=self._on_select)
        self._prev_dd = value

    def _apply_filter(self, instance, text):
        if self.dropdown:
            print("ENTOR")
            self.dropdown.apply_filter(text)

    def _on_release(self, *largs):
        if not self.dropdown:
            return
        self.remove_widget(self._button)
        self.add_widget(self._textinput)
        self.dropdown.open(self)
        self._textinput.focus = True

    def _on_dismiss(self, *largs):
        self.remove_widget(self._textinput)
        self.add_widget(self._button)
        self._textinput.text = ''

    def _on_select(self, instance, value):
        self.text = value

    def _on_enter(self, *largs):
        container = self.dropdown.container
        if container.children:
            self.dropdown.select(container.children[-1].text)
        else:
            self.dropdown.dismiss()

class Prueba(App):
    def build(self):
        self.box = BoxLayout(orientation = 'vertical')
        F= FilterDDTrigger(orientation = 'vertical')
        F.text = 'Spanish numbers'
        F.dropdown = FilterDD(options = [i for i in ['uno','dos','tres']])
        self.box.add_widget(F )
        for i in range(5):
            button = Button(text='Hello world', font_size=14)
            button.bind(on_press = exit)
            self.box.add_widget(button)        
        return self.box

if __name__ == '__main__':
    Prueba().run()
