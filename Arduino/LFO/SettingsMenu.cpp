#include "SettingsMenu.h"

void SettingsItem::setNext(SettingsItem* next) {
  if (_next) {
    next->_next = _next;
    _next->_previous = next;
  }
  _next = next;
  next->_previous = this;
  next->_parent = _parent;
}

SettingsItem* SettingsItem::head() const {
  SettingsItem* prev = const_cast<SettingsItem*>(this);
  while (prev->_previous) {
    prev = prev->_previous;
  }
  return prev;
}

SettingsItem* SettingsItem::tail() const {
  SettingsItem* next = const_cast<SettingsItem*>(this);
  while (next->_next) {
    next = next->_next;
  }
  return next;
}

SettingsItem* SettingsItem::get(int pos) const {
  SettingsItem* sibling = const_cast<SettingsItem*>(head());
  while (pos > 0 && sibling->_next) {
    sibling = sibling->_next;
    pos--;
  }
  return sibling;
}

int SettingsItem::countSiblings() const {
  int count = 1;
  const SettingsItem* sibling = head();
  while (sibling = sibling->_next) {
    ++count;
  }
  return count;
}

void SettingsItem::addChild(SettingsItem* child) {
  if (_firstChild) {
    _firstChild->tail()->setNext(child);
  }
  else {
    _firstChild = child;
    child->_parent = this;
  }
}

void SettingsItem::destroy() {
  while (SettingsItem* child = getChild(0)) {
    child->destroy();
  }
  
  if (_previous) {
    if (_next) {
      _previous->setNext(_next);
    }
  }
  else {
    if (_next) {
      _next->_previous = nullptr;
      _parent->_firstChild = _next;
    }
    else {
      _parent->_firstChild = nullptr;
    }
  }
  
  delete this;
}

SettingsItem* SettingsItem::getChild(int pos) const {
  return _firstChild ? _firstChild->get(pos) : nullptr;
}

int SettingsItem::countChildren() const {
  return _firstChild ? _firstChild->countSiblings() : 0;
}

int SettingsItem::position() const {
  int pos = 0;
  const SettingsItem* prev = this;
  while (prev->_previous) {
    prev = prev->_previous;
    pos++;
  }
  return pos;
}

IntSetting::IntSetting(const char* name, int& val, int minimum, int maximum, int def)
: Setting(name, val, def)
, _minimum(minimum)
, _maximum(maximum) {}

bool IntSetting::setValue(int val) {
  return Setting::setValue(val > _maximum ? _maximum : (val < _minimum ? _minimum : val));
}
  
ChoiceSetting::ChoiceSetting(const char* name, int& val, const char* values[], int count, int def)
: IntSetting(name, val, 0, count - 1, def)
, _values(values) {}

const char* SettingsMenu::BuiltInAction::_actionNames[4] = { "Back", "Save & Exit", "Cancel", "Defaults" };

SettingsMenu::BuiltInAction::BuiltInAction(SettingsMenu* settingsMenu, Type type)
: Action(_actionNames[static_cast<int>(type)])
, _settingsMenu(settingsMenu)
, _type(type) {}

void SettingsMenu::BuiltInAction::perform() {
  switch(_type) {
  case Type::kBack:
    _settingsMenu->navigateBack();
    break;
    
  case Type::kSaveAndExit:
    _settingsMenu->saveAndExit();
    break;
    
  case Type::kCancel:
    _settingsMenu->cancel();
    break;
    
  case Type::kResetToDefaults:
    _settingsMenu->resetToDefaults();
    break;
  }
}

SettingsMenu::SettingsMenu(DisplayInterface* display, RotaryButton& rotary)
: Menu("Settings")
, _display(display)
, _rotary(rotary) {
  display->requestFontHeight(kFontHeight);
  _maxLines = display->height() / display->fontHeight();
}

void SettingsMenu::show() {
  if (_isHidden) {
    _rotary.saveState(_savedRotaryState);
    setContext(this);
  }

  if (!_context || !_selectedItem) {
    return;
  }
  
  switch(_context->type()) {
  case SettingsItem::Type::kMenu:
    displayMenu(static_cast<const Menu*>(_context));
    break;
    
  case SettingsItem::Type::kChoice:
    displayChoiceSetting(static_cast<const ChoiceSetting*>(_context));
    break;

  default:
    return;
  }

  _isHidden = false;
  _display->show();
}

void SettingsMenu::hide() {
  setContext(nullptr);
  _display->clear();
  _isHidden = true;
  _isDirty = false;
  _rotary.reset();
  _rotary.restoreState(_savedRotaryState);
}

void SettingsMenu::saveAndExit() {
  if (_isDirty && _saveCallback) {
    _saveCallback();
  }
  hide();
  if (_onHiddenAfterSaveCallback) {
    _onHiddenAfterSaveCallback();
  }
}

void SettingsMenu::cancel() {
  if (_isDirty && _loadCallback) {
    _loadCallback();
  }
  hide();
  if (_onHiddenAfterCancelCallback) {
    _onHiddenAfterCancelCallback();
  }
}

Action* SettingsMenu::createBackAction() {
  return new BuiltInAction(this, BuiltInAction::Type::kBack);
}

Action* SettingsMenu::createSaveAndExitAction(Callback saveCallback, Callback onHiddenCallback) {
  _saveCallback = saveCallback;
  _onHiddenAfterSaveCallback = onHiddenCallback;
  return new BuiltInAction(this, BuiltInAction::Type::kSaveAndExit);
}

Action* SettingsMenu::createCancelAction(Callback loadCallback, Callback onHiddenCallback) {
  _loadCallback = loadCallback;
  _onHiddenAfterCancelCallback = onHiddenCallback;
  return new BuiltInAction(this, BuiltInAction::Type::kCancel);  
}

Action* SettingsMenu::createResetToDefaultsAction() {
  return new BuiltInAction(this, BuiltInAction::Type::kResetToDefaults);
}

void SettingsMenu::updateRotary() {
  if (_isHidden) {
    show();
    _rotary.reset();
    return;
  }
  
  applyTurns();
  applyButton();
  if (!_isHidden) {
    show();
  }
}

void SettingsMenu::navigateBack() {
  if (_editing) {
    _editing = false;
    navigateToItem(_selectedItem);
    _rotary.setCounterRange(0, _context->countChildren() - 1);
    _rotary.resetCounter(_selectedItem->position());
  }
  else {
    navigateToItem(_selectedItem->_parent);
  }
}
  
void SettingsMenu::navigateToItem(SettingsItem* item) {
  if (item == this) {
    setContext(this);
    return;
  }

  if (!item || !item->_parent) {
    return;
  }
  
  setContext(item->_parent);
  _selectedItem = item;
  scrollToPos(item->position());
}

void SettingsMenu::setContext(SettingsItem* context) {
  if (context == _context) {
    return;
  }

  if (!context) {
    _context = nullptr;
    return;
  }
  
  switch (context->type()) {
  case SettingsItem::Type::kMenu:
    {
      const Menu* menu = static_cast<const Menu*>(context);
      _rotary.setCounterRange(0, menu->countChildren() - 1);
      _rotary.resetCounter();
      _editing = false;
      _selectedItem = menu->_firstChild;
      _contextScrollPos = 0;
    }
    break;
    
  case SettingsItem::Type::kChoice:
    {
      const ChoiceSetting* setting = static_cast<const ChoiceSetting*>(context);
      _rotary.setCounterRange(setting->minimum(), setting->maximum() + 1);
      _rotary.resetCounter(setting->value());
      _editing = true;
      _selectedItem = context;
      _contextScrollPos = 0;
      scrollToPos(setting->value());
    }
    break;

  default:
    return;
  }
  
  _context = context;
}

void SettingsMenu::scrollToPos(int pos) {
  if (pos >= _contextScrollPos + _maxLines) {
    _contextScrollPos = pos - _maxLines + 1;
  }
  else if (pos < _contextScrollPos) {
    _contextScrollPos = pos;
  }
}

void SettingsMenu::applyTurns() {
  switch (_context->type()) {
  case SettingsItem::Type::kMenu:
    if (_editing) {
      IntSetting* setting = static_cast<IntSetting*>(_selectedItem);
      if (setting->setValue(_rotary.counter())) {
        onSettingChanged(setting);
      }
    }
    else {
      navigateToItem(_context->getChild(_rotary.counter()));
    }
    break;
    
  case SettingsItem::Type::kChoice:
    scrollToPos(_rotary.counter());
    break;
    
  default:
    break;
  }
}

void SettingsMenu::applyButton() {
  if (_rotary.triggered(SINGLE_TAP)) {
    if (_editing) {
      if (_selectedItem->type() == SettingsItem::Type::kChoice) {
        ChoiceSetting* setting = static_cast<ChoiceSetting*>(_selectedItem);
        if (_rotary.counter() <= setting->maximum()) {
          if (setting->setValue(_rotary.counter())) {
            onSettingChanged(setting);
          }
        }
      }
      navigateBack();
    }
    else {
      switch(_context->type()) {
      case SettingsItem::Type::kMenu:
        applyMenuTap();
        break;

      default:
        break;
      }
    }
  }
}
  
void SettingsMenu::applyMenuTap() {
  switch(_selectedItem->type()) {
  case SettingsItem::Type::kMenu:
  case SettingsItem::Type::kChoice:
    setContext(_selectedItem);
    break;

  case SettingsItem::Type::kAction:
    {
      Action* action = static_cast<Action*>(_selectedItem);
      action->perform();
    }
    break;

  case SettingsItem::Type::kInt:
    {
      const IntSetting* setting = static_cast<const IntSetting*>(_selectedItem);
      _rotary.setCounterRange(setting->minimum(), setting->maximum());
      _rotary.resetCounter(setting->value());
      _editing = true;
      break;
    }
  }
}

void SettingsMenu::onSettingChanged(SettingBase* setting) {
  _isDirty = true;
  if (_settingChangedCallback) {
    _settingChangedCallback(setting);
  }
}

void SettingsMenu::displayMenu(const Menu* menu) {
  SettingsItem* item = menu->_firstChild->get(_contextScrollPos);
  for (int i = 0; i < _maxLines && item; i++) {
    int y = i * _display->fontHeight();
    
    if (_selectedItem == item && !_editing) {
      _display->drawString(0, y, ">");
    }
    _display->drawString(kInset, y, item->name());
    
    if (_selectedItem == item && _editing) {
      _display->drawString(_display->width(), y, String("< ") + item->description() + " >", Align::kRight);
    }
    else {
      _display->drawString(_display->width(), y, item->description(), Align::kRight);
    }
    item = item->_next;
  }
}

void SettingsMenu::displayChoiceSetting(const ChoiceSetting* choice) {
  _display->drawString(kInset, 0, choice->name());
  int index = _contextScrollPos;
  for (int i = 0; i < _maxLines && index <= _rotary.counterMaximum(); i++) {
    int y = i * kFontHeight;
    if (index <= choice->maximum()) {
      String s = choice->toString(index) + (index == choice->value() ? " [x]" : " [  ]");
      _display->drawString(_display->width() - kInset, y, s, Align::kRight);
    }
    else {
      _display->drawString(_display->width() - kInset, y, "Back", Align::kRight);
    }
    if (index == _rotary.counter()) {
      _display->drawString(_display->width(), y, "<", Align::kRight);
    }
    index++;
  }
}

void SettingsMenu::resetToDefaults() {
  resetToDefaults(this);
}

void SettingsMenu::resetToDefaults(SettingsItem* item) {
  switch (item->type()) {
  case SettingsItem::Type::kInt:
  case SettingsItem::Type::kChoice:
    {
      SettingBase* setting = static_cast<SettingBase*>(item);
      if (setting->reset()) {
        onSettingChanged(setting);
      }
    }
    break;

  default:
    {
      int children = item->countChildren();
      for (int i = 0; i < children; i++) {
        resetToDefaults(item->getChild(i));
      }
    }
    break;
  }
}

