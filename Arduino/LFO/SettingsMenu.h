#pragma once

#include "DisplayInterface.h"
#include "RotaryButton.h"

class SettingsItem {
public:
  enum class Type {
    kAction,
    kMenu,
    kInt,
    kChoice,
  } _type;

  SettingsItem(const char* name) : _name(name) {}

  String name() const { return _name; }
  virtual String description() const = 0;
  virtual Type type() const = 0;

  void addChild(SettingsItem* child);
  void setNext(SettingsItem* next);
  SettingsItem* head() const;
  SettingsItem* tail() const;
  SettingsItem* get(int pos) const;
  SettingsItem* getChild(int pos) const;
  int countSiblings() const;
  int countChildren() const;
  int position() const;
  void destroy();

  SettingsItem* _parent = 0;
  SettingsItem* _previous = 0;
  SettingsItem* _next = 0;
  SettingsItem* _firstChild = 0;
  
private:
  const char* _name;
};

class Action : public SettingsItem {
public:
  Action(const char* name)
  : SettingsItem(name) {}
  
  String description() const override { return ""; }
  Type type() const override { return Type::kAction; }

  virtual void perform() = 0;
};

class Menu : public SettingsItem {
public:
  Menu(const char* name)
  : SettingsItem(name) {}
  
  String description() const override { return "..."; }
  Type type() const override { return Type::kMenu; }
};

class SettingBase : public SettingsItem {
public:
  SettingBase(const char* name) : SettingsItem(name) {}

  virtual bool reset() = 0;
};

template<typename T>
class Setting : public SettingBase {
public:
  Setting(const char* name, T& val, T def)
  : SettingBase(name)
  , _value(val)
  , _default(def) {}

  String description() const override { return toString(); }
  T value() const { return _value; }
  T defaultValue() const { return _default; }
  virtual bool setValue(T val) {
    if (val == _value) {
      return false;
    }
    _value = val;
    return true;
  }
  bool reset() override { return setValue(_default); }
  String toString() const { return toString(value()); }
  virtual String toString(T val) const { return String(val); }

private:
  T& _value;
  T _default;
};

class IntSetting : public Setting<int> {
public:
  IntSetting(const char* name, int& val, int minimum, int maximum, int def);

  Type type() const override { return Type::kInt; }
  bool setValue(int val) override;
  int minimum() const { return _minimum; }
  int maximum() const { return _maximum; }
  
private:
  int _minimum;
  int _maximum;
};

class ChoiceSetting : public IntSetting {
public:
  ChoiceSetting(const char* name, int& val, const char* values[], int count, int def);

  Type type() const override { return Type::kChoice; }
  String toString(int val) const override { return _values[val]; }

private:
  const char** _values;
};

class SettingsMenu : public Menu {
public:

  class DisplayInterface {
  public:
    virtual void clear() = 0;
    virtual void show() = 0;
    virtual int width() = 0;
    virtual int height() = 0;
    virtual void requestFontHeight(int hgt) = 0;
    virtual int fontHeight() = 0;
    virtual void drawString(int x, int y, String s, Align align = Align::kLeft) = 0;
  };

  typedef void (*Callback)();
  typedef void (*SettingChangedCallback)(SettingBase*);
  
  SettingsMenu(DisplayInterface* display, RotaryButton& rotary);
  
  void show();
  void hide();
  bool isVisible() { return !_isHidden; }
  void updateRotary();
  void navigateBack();
  void navigateToStart() { navigateToItem(this); }
  void navigateToItem(SettingsItem* item);
  void resetToDefaults();
  void setSettingChangedCallback(SettingChangedCallback callback) { _settingChangedCallback = callback; }
  void onSettingChanged(SettingBase* setting);
  
  Action* createBackAction();
  Action* createSaveAndExitAction(Callback saveCallback, Callback onHiddenCallback = nullptr);
  Action* createCancelAction(Callback loadCallback, Callback onHiddenCallback = nullptr);
  Action* createResetToDefaultsAction();

protected:
  void setContext(SettingsItem* context);
  void scrollToPos(int pos);
  void applyTurns();
  void applyButton();
  void applyMenuTap();
  void displayMenu(const Menu* menu);
  void displayChoiceSetting(const ChoiceSetting* choice);
  void saveAndExit();
  void cancel();
  void resetToDefaults(SettingsItem* item);
  
private:
  class BuiltInAction : public Action {
  public:
    enum class Type {
      kBack = 0,
      kSaveAndExit,
      kCancel,
      kResetToDefaults,
    };

    BuiltInAction(SettingsMenu* settingsMenu, Type type);

    void perform() override;

  private:
    static const char* _actionNames[4];
  
    SettingsMenu* _settingsMenu;
    Type _type;
  };

  static constexpr int kFontHeight = 16;
  static constexpr int kInset = 10;

  DisplayInterface* _display;
  RotaryButton& _rotary;
  SettingsItem* _context = nullptr;
  SettingsItem* _selectedItem = nullptr;
  int _contextScrollPos = 0;
  bool _editing = false;
  int _maxLines;
  bool _isHidden = true;
  RotaryButton::State _savedRotaryState;
  Callback _saveCallback = nullptr;
  Callback _loadCallback = nullptr;
  Callback _onHiddenAfterSaveCallback = nullptr;
  Callback _onHiddenAfterCancelCallback = nullptr;
  SettingChangedCallback _settingChangedCallback = nullptr;
  bool _isDirty = false;
};
