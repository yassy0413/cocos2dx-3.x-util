#include "CCDebugMenu.h"

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
	target->_container = this;
    
    if( onValueChanged ){
        target->onValueChenged = onValueChanged;
    }
    
    _components.push_back(target);
}

#pragma mark -- Component

DebugMenu::Component::Component()
: _container(nullptr)
, onValueChenged([](Component* sender){})
{}

DebugMenu::Component::~Component()
{}

bool DebugMenu::Component::initWithKey(const std::string& key){
    if( Node::init() ){
        _key = key;
        return true;
    }
    return false;
}

#pragma mark -- Component Preset

bool DebugMenu::Component::Preset::Flag::initWithKey(const std::string& key){
    if( Component::initWithKey(key) ){
        setContentSize( Size( Director::getInstance()->getWinSize().width * 0.8f, 40 ) );
        
        auto background = LayerColor::create( Color4B(0, 50, 0, 150), getContentSize().width, getContentSize().height );
        addChild( background );
        
        auto text = Label::createWithSystemFont(_key, "arial", 18);
        text->setAnchorPoint( Point( 0.0f, 0.5f ) );
        text->setPosition( Point(8.0f, getContentSize().height * 0.5f) );
        addChild(text);
        
        auto maskSprite = Sprite::create("images/debug/switch-mask.png");
        auto control = extension::ControlSwitch::create(maskSprite,
                                                        Sprite::create("images/debug/switch-on.png"),
                                                        Sprite::create("images/debug/switch-off.png"),
                                                        Sprite::create("images/debug/switch-thumb.png"),
                                                        Label::createWithSystemFont("on", "Arial", 12),
                                                        Label::createWithSystemFont("off", "Arial", 12) );
        control->setAnchorPoint( Point( 0.0f, 0.5f ) );
        control->setPosition( Point(getContentSize().width - maskSprite->getContentSize().width - 8.0f, getContentSize().height * 0.5f) );
        {
            control->setOn( UserDefault::getInstance()->getBoolForKey( _key.c_str() ) );
        }
        control->addTargetWithActionForControlEvents( this, cccontrol_selector(Flag::onControlEvent), extension::Control::EventType::VALUE_CHANGED );
        addChild(control);
        
        return true;
    }
    return false;
}

void DebugMenu::Component::Preset::Flag::onControlEvent(Ref* sender, extension::Control::EventType controlEvent){
	auto p = (extension::ControlSwitch*)sender;
	UserDefault::getInstance()->setBoolForKey( _key.c_str(), p->isOn() );
    onValueChenged(this);
}

bool DebugMenu::Component::Preset::Button::initWithKey(const std::string& key){
    if( Component::initWithKey(key) ){
        setContentSize( Size( Director::getInstance()->getWinSize().width * 0.8f, 32 ) );
        
        auto background = LayerColor::create( Color4B(0, 50, 0, 150), getContentSize().width, getContentSize().height );
        addChild( background );
        
        auto control = extension::ControlButton::create(Label::createWithSystemFont(_key.c_str(), "arial", 18),
                                                        ui::Scale9Sprite::create("images/debug/button.png") );
        control->setAnchorPoint( Point( 0.5f, 0.5f ) );
        control->setPosition( Point(getContentSize().width * 0.5f, getContentSize().height * 0.5f) );
        {
            control->setPreferredSize( Size(getContentSize().width * 0.8f, getContentSize().height * 0.8f) );
        }
        control->addTargetWithActionForControlEvents( this, cccontrol_selector(Button::onControlEvent), extension::Control::EventType::TOUCH_UP_INSIDE );
        addChild(control);
        
        return true;
    }
    return false;
}

void DebugMenu::Component::Preset::Button::onControlEvent(Ref* sender, extension::Control::EventType controlEvent){
	CC_ASSERT(_container);
    onValueChenged(this);
}

bool DebugMenu::Component::Preset::Slider::initWithKey(const std::string& key){
    if( Component::initWithKey(key) ){
        setContentSize( Size( Director::getInstance()->getWinSize().width * 0.8f, 32 ) );
        
        auto background = LayerColor::create( Color4B(0, 50, 0, 150), getContentSize().width, getContentSize().height );
        addChild( background );
        
        _label = Label::createWithSystemFont( "", "arial", 18 );
        _label->setAnchorPoint( Point( 0.0f, 0.5f ) );
        _label->setPosition( Point(8.0f, getContentSize().height * 0.5f) );
        addChild(_label);
        updateLabel();
        
        auto control = extension::ControlSlider::create("images/debug/sliderTrack.png",
                                                        "images/debug/sliderProgress.png",
                                                        "images/debug/sliderThumb.png");
        control->setAnchorPoint( Point( 0.0f, 0.5f ) );
        control->setPosition( Point(128, getContentSize().height * 0.5f) );
        {
            control->setMinimumValue( _minValue );
            control->setMaximumValue( _maxValue );
            control->setValue( UserDefault::getInstance()->getFloatForKey( _key.c_str() ) );
            control->setContentSize( Size(getContentSize().width * 0.8f, getContentSize().height * 0.8f) );//TODO!
        }
        control->addTargetWithActionForControlEvents( this, cccontrol_selector(Slider::onControlEvent), extension::Control::EventType::VALUE_CHANGED );
        addChild(control);
        
        return true;
    }
    return false;
}

void DebugMenu::Component::Preset::Slider::onControlEvent(Ref* sender, extension::Control::EventType controlEvent){
	auto p = (extension::ControlSlider*)sender;
	UserDefault::getInstance()->setFloatForKey( _key.c_str(), p->getValue() );
	updateLabel();
    onValueChenged(this);
}

void DebugMenu::Component::Preset::Slider::updateLabel(){
	char buf[256];
	sprintf( buf, "%s %.1f", _key.c_str(), UserDefault::getInstance()->getFloatForKey( _key.c_str() ) );
	_label->setString(buf);
}

NS_CC_EXT_END
