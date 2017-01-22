#include "CCDebugMenu.h"

#if COCOS2D_DEBUG > 0
NS_CC_EXT_BEGIN

#pragma mark -- Debug Menu

// singleton stuff
static DebugMenu *s_SharedDebugMenu = nullptr;

DebugMenu* DebugMenu::getInstance()
{
    if (!s_SharedDebugMenu)
    {
        s_SharedDebugMenu = new (std::nothrow) DebugMenu();
        CCASSERT(s_SharedDebugMenu, "FATAL: Not enough memory");
        s_SharedDebugMenu->init();
    }
    return s_SharedDebugMenu;
}

DebugMenu::DebugMenu()
: _rootContainer(nullptr)
, _background(nullptr)
, _thresholdAccele(0.0f)
, _thresholdLongTap(0.0f)
, _touching(false)
{}

DebugMenu::~DebugMenu(){
    _rootContainer->release();
    _background->release();
    Director::getInstance()->getScheduler()->unscheduleUpdate(this);
}

bool DebugMenu::init(){
    _background = new (std::nothrow) LayerColor();
    _background->initWithColor(Color4B(0, 0, 0, 50),
                               Director::getInstance()->getWinSize().width,
                               Director::getInstance()->getWinSize().height);
    
    _rootContainer = new (std::nothrow) Container();
    _rootContainer->add( new (std::nothrow) Component::Preset::Button("Close"), [](Component* sender){
        DebugMenu::getInstance()->close();
    });
    _rootContainer->add( new (std::nothrow) Component::Preset::Flag("Debug Info"), [](Component* sender){
        const auto var = UserDefault::getInstance()->getBoolForKey(sender->getKey().c_str());
        Director::getInstance()->setDisplayStats(var);
    });
    _rootContainer->add( new (std::nothrow) Component::Preset::Slider("DT", 0.0f, 4.0f), [](Component* sender){
        const auto var = UserDefault::getInstance()->getFloatForKey(sender->getKey().c_str());
        Director::getInstance()->getScheduler()->setTimeScale(var);
    });
    return true;
}

void DebugMenu::enableLongTap(float seconds){
    CC_ASSERT(seconds > 0.0f);
    if( _thresholdLongTap <= 0.0f ){
        // タッチイベントを利用する
        auto listener = EventListenerTouchOneByOne::create();
        listener->onTouchBegan = [this](Touch* touch, Event* event){
            _touchBeganTime = std::chrono::system_clock::now();
            _touchBeganPoint = touch->getLocation();
            _touchCurrentPoint = _touchBeganPoint;
            _touching = true;
            return true;
        };
        listener->onTouchMoved = [this](Touch* touch, Event* event){
            _touchCurrentPoint = touch->getLocation();
        };
        listener->onTouchEnded =
        listener->onTouchCancelled = [this](Touch* touch, Event* event){
            _touching = false;
        };
        auto eventDispatcher = Director::getInstance()->getEventDispatcher();
        eventDispatcher->addEventListenerWithFixedPriority(listener, 0xffff);
        
        Director::getInstance()->getScheduler()->scheduleUpdate(this, 0.0f, false);
    }
    _thresholdLongTap = seconds;
}

void DebugMenu::enableKeyboard(EventKeyboard::KeyCode keycode){
    auto listener = cocos2d::EventListenerKeyboard::create();
    listener->onKeyReleased = [this, keycode](cocos2d::EventKeyboard::KeyCode kc, cocos2d::Event* event){
        if( keycode == kc ){
            if( _rootContainer->getParent() ){
                close();
            }else{
                open();
            }
        }
    };
    cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(listener, 0xffff);
}

void DebugMenu::enableAccele(float threshold){
    CC_ASSERT(threshold > 0.0f);
    if( _thresholdAccele <= 0.0f ){
        // 加速度センサーを利用する
        Device::setAccelerometerEnabled(true);
        auto listener = EventListenerAcceleration::create( [this](Acceleration* acc, Event* event) {
            Vec3 vec( acc->x, acc->y, acc->z );
            //CCLOG("Accele [%.1f,%.1f,%.1f] %f", vec.x, vec.y, vec.z, kmVec3Length(&vec));
            if( vec.length() > _thresholdAccele ){
                open();
            }
        });
        auto eventDispatcher = Director::getInstance()->getEventDispatcher();
        eventDispatcher->addEventListenerWithFixedPriority(listener, 0xffff);
    }
    _thresholdAccele = threshold;
}

void DebugMenu::update(float delta){
	// ロングタップ検出
	if( _touching ){
        const auto elapsed = std::chrono::system_clock::now() - _touchBeganTime;
        const float duration = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.0f;
		if( duration >= _thresholdLongTap ){
			if( _touchCurrentPoint.distance(_touchBeganPoint) < 12.0f ){
				open();
			}
            _touching = false;
		}
	}
}

void DebugMenu::open(){
    if( !_rootContainer->getParent() ){
        auto runningScene = Director::getInstance()->getRunningScene();
        runningScene->addChild(_background);
        runningScene->addChild(_rootContainer);
    }
}

void DebugMenu::close(){
    if( _rootContainer->getParent() ){
        _rootContainer->removeFromParent();
        _background->removeFromParent();
    }
    _touching = false;
}

#pragma mark -- Container

DebugMenu::Container::Container()
: _scrollView(nullptr)
{
    setContentSize( Director::getInstance()->getWinSize() );
    
    // Touch Guard
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches( true );
    listener->onTouchBegan = [this](Touch* touch, Event* event){
        return true;
    };
    listener->onTouchMoved =
    listener->onTouchEnded =
    listener->onTouchCancelled = [this](Touch* touch, Event* event){};
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    //
    _scrollView = ScrollView::create( getContentSize() );
    _scrollView->setBounceable(true);
    _scrollView->setDirection(ScrollView::Direction::VERTICAL);
    addChild(_scrollView);
}

DebugMenu::Container::~Container(){
    for( auto component : _components ){
        component->release();
    }
}

void DebugMenu::Container::onEnter(){
	Node::onEnter();
	
    //
    _offsetY = 0.0f;
    for( auto component : _components ){
        if( !component->getParent() ){
            _scrollView->addChild(component);
        }
        
        const float marginY = 4.0f;
        const float halfH = ( component->getContentSize().height + marginY ) * 0.5f;
        
        _offsetY += halfH;
        component->setAnchorPoint( Point( 0.5f, 0.5f ) );
        component->setPositionX( _scrollView->getContentSize().width * 0.5f );
        component->setPositionY( _offsetY );
        _offsetY += halfH;
        
        //
        const Size size( _scrollView->getContentSize() );
        _scrollView->setContentSize( Size( size.width, std::max( size.height, _offsetY ) ) );
        _scrollView->setContentOffset( Point( 0.0f, std::min( _scrollView->getViewSize().height - _offsetY, 0.0f ) ) );
    }
}

void DebugMenu::Container::onExit(){
	Node::onExit();
}

void DebugMenu::Container::add(Component* target, Component::OnValueChenged onValueChanged){
	
	CC_ASSERT(!target->_container);
    target->setContainer(this);
    
    if( onValueChanged ){
        target->onValueChenged = onValueChanged;
    }
    
    _components.push_back(target);
}

#pragma mark -- Component

DebugMenu::Component::Component(const std::string& key)
: _key(key)
, _container(nullptr)
, onValueChenged([](Component* sender){})
{
    Node::init();
}

DebugMenu::Component::~Component()
{}

void DebugMenu::Component::setContainer(cocos2d::extension::DebugMenu::Container* container){
    _container = container;
}

#pragma mark -- Component Preset

void DebugMenu::Component::Preset::Flag::setContainer(Container* container){
    Component::setContainer(container);
    
    const Size& winSize = Director::getInstance()->getWinSize();
    
    setContentSize(Size(winSize.width * 0.8f, winSize.height * 0.08f));
    
    auto background = LayerColor::create( Color4B(0, 50, 0, 150), getContentSize().width, getContentSize().height );
    addChild( background );
    
    auto text = Label::createWithSystemFont(_key, "arial", winSize.height * 0.03f);
    text->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    text->setPosition( Point(8.0f, getContentSize().height * 0.5f) );
    addChild(text);
    
    const std::string str = UserDefault::getInstance()->getBoolForKey(_key.c_str())?"ON":"OFF";
    auto control = extension::ControlButton::create(Label::createWithSystemFont(str, "arial", winSize.height * 0.03f),
                                                    ui::Scale9Sprite::create("images/debug/button.png") );
    control->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    control->setPositionX(getContentSize().width * 0.85f);
    control->setPositionY(getContentSize().height * 0.5f);
    control->setPreferredSize( Size(getContentSize().width * 0.25f, getContentSize().height * 0.8f) );
    control->addTargetWithActionForControlEvents(this, cccontrol_selector(Flag::onControlEvent), extension::Control::EventType::TOUCH_UP_INSIDE);
    addChild(control);
}

void DebugMenu::Component::Preset::Flag::onControlEvent(Ref* sender, extension::Control::EventType controlEvent){
    CC_ASSERT(_container);
	auto p = (extension::ControlButton*)sender;
	UserDefault::getInstance()->setBoolForKey(_key.c_str(), !UserDefault::getInstance()->getBoolForKey(_key.c_str()));
    UserDefault::getInstance()->flush();
    const std::string str = UserDefault::getInstance()->getBoolForKey(_key.c_str())?"ON":"OFF";
    p->setTitleForState(str, extension::Control::State::NORMAL);
    onValueChenged(this);
}

void DebugMenu::Component::Preset::Button::setContainer(Container* container){
    Component::setContainer(container);
    
    const Size& winSize = Director::getInstance()->getWinSize();
    
    setContentSize(Size(winSize.width * 0.8f, winSize.height * 0.08f));
    
    auto background = LayerColor::create( Color4B(0, 50, 0, 150), getContentSize().width, getContentSize().height );
    addChild( background );
    
    auto control = extension::ControlButton::create(Label::createWithSystemFont(_key.c_str(), "arial", winSize.height * 0.03f),
                                                    ui::Scale9Sprite::create("images/debug/button.png") );
    control->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    control->setPosition(getContentSize() * 0.5f);
    control->setPreferredSize(getContentSize() * 0.8f);
    control->addTargetWithActionForControlEvents(this, cccontrol_selector(Button::onControlEvent), extension::Control::EventType::TOUCH_UP_INSIDE);
    control->setTag(0xff);
    addChild(control);
}

void DebugMenu::Component::Preset::Button::onControlEvent(Ref* sender, extension::Control::EventType controlEvent){
	CC_ASSERT(_container);
    onValueChenged(this);
}

void DebugMenu::Component::Preset::Button::onEnter(){
    Component::onEnter();
    if( auto control = reinterpret_cast<extension::ControlButton*>(getChildByTag(0xff)) ){
        control->setHighlighted(false);
    }
}

void DebugMenu::Component::Preset::Slider::setContainer(Container* container){
    Component::setContainer(container);
    
    const Size& winSize = Director::getInstance()->getWinSize();
    
    setContentSize(Size(winSize.width * 0.8f, winSize.height * 0.08f));
    
    auto background = LayerColor::create(Color4B(0, 50, 0, 150), getContentSize().width, getContentSize().height);
    addChild( background );
    
    _label = Label::createWithSystemFont("", "arial", winSize.height * 0.03f);
    _label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    _label->setPosition(8.0f, getContentSize().height * 0.5f);
    addChild(_label);
    updateLabel();
    
    auto control = extension::ControlSlider::create("images/debug/sliderTrack.png",
                                                    "images/debug/sliderProgress.png",
                                                    "images/debug/sliderThumb.png");
    control->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    control->setPosition(_label->getPositionX() + _label->getContentSize().width + getContentSize().width * 0.1f, getContentSize().height * 0.5f);
    {
        control->setMinimumValue(_minValue);
        control->setMaximumValue(_maxValue);
        control->setValue(UserDefault::getInstance()->getFloatForKey( _key.c_str() ));
        control->setScaleX((getContentSize().width - _label->getContentSize().width) / control->getContentSize().width * 0.8f);
        control->setScaleY(getContentSize().height * 0.8f / control->getContentSize().height);
    }
    control->addTargetWithActionForControlEvents(this, cccontrol_selector(Slider::onControlEvent), extension::Control::EventType::VALUE_CHANGED);
    addChild(control);
}

void DebugMenu::Component::Preset::Slider::onControlEvent(Ref* sender, extension::Control::EventType controlEvent){
	auto p = (extension::ControlSlider*)sender;
	UserDefault::getInstance()->setFloatForKey( _key.c_str(), p->getValue() );
    UserDefault::getInstance()->flush();
	updateLabel();
    onValueChenged(this);
}

void DebugMenu::Component::Preset::Slider::updateLabel(){
	char buf[256];
    const float value = UserDefault::getInstance()->getFloatForKey( _key.c_str() );
    sprintf( buf, "%s %.1f", _key.c_str(), value );
	_label->setString(buf);
}

void DebugMenu::Component::Preset::SliderI::updateLabel(){
    char buf[256];
    const float value = UserDefault::getInstance()->getFloatForKey( _key.c_str() );
    sprintf( buf, "%s %d", _key.c_str(), static_cast<int32_t>(value) );
    _label->setString(buf);
}

NS_CC_EXT_END
#endif
