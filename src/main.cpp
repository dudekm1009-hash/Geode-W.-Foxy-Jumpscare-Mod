#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <random>

using namespace geode::prelude;

// ====== USTAWIENIA ======
constexpr bool TEST_ON_START = true; // true = test (od razu), false = final (1/1000)
constexpr int RANDOM_DENOMINATOR = 1000; // 1/1000
constexpr float CHECK_INTERVAL = 1.0f;  // co ile sekund sprawdzamy losowość
constexpr float ANIM_FPS = 25.0f;        // animacja jumpscare
constexpr float SHOW_TIME = 0.55f;       // ile trwa jumpscare
// =======================

class $modify(PlayLayer) {
    bool started = false;
    bool cooldown = false;

    void startRandomTimer() {
        this->schedule(schedule_selector(PlayLayer::tick), CHECK_INTERVAL);
    }

    void tick(float) {
        if (cooldown) return;

        static std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
        std::uniform_int_distribution<int> dist(1, RANDOM_DENOMINATOR);
        if (dist(rng) == 1) {
            doJumpscare();
        }
    }

    void doJumpscare() {
        if (cooldown) return;
        cooldown = true;

        auto win = CCDirector::sharedDirector()->getWinSize();

        // Sprite startowy (pierwsza klatka)
        auto sprite = CCSprite::create("foxy-1.png");
        sprite->setAnchorPoint({0.5f, 0.5f});
        sprite->setPosition(win / 2);
        sprite->setZOrder(9999);

        // Fullscreen scale (bez pasków)
        float sx = win.width  / sprite->getContentSize().width;
        float sy = win.height / sprite->getContentSize().height;
        sprite->setScale(std::max(sx, sy));

        this->addChild(sprite);

        // Dźwięk
        FMODAudioEngine::sharedEngine()->playEffect("scream.ogg");

        // Animacja klatkowa
        Vector<CCSpriteFrame*> frames;
        for (int i = 1; i <= 14; i++) {
            auto name = fmt::format("foxy-{}.png", i);
            auto tex = CCTextureCache::sharedTextureCache()->addImage(name.c_str());
            if (!tex) continue;
            auto fr = CCSpriteFrame::createWithTexture(
                tex, CCRect(0, 0, tex->getContentSize().width, tex->getContentSize().height)
            );
            frames.pushBack(fr);
        }
        auto anim = CCAnimation::createWithSpriteFrames(frames, 1.0f / ANIM_FPS);
        sprite->runAction(CCAnimate::create(anim));

        // Zniknięcie i cooldown
        sprite->runAction(CCSequence::create(
            CCDelayTime::create(SHOW_TIME),
            CCFadeOut::create(0.15f),
            CCCallFuncN::create(sprite, callfuncN_selector(CCNode::removeFromParent)),
            nullptr
        ));

        // Krótki cooldown, żeby nie spamowało
        this->runAction(CCSequence::create(
            CCDelayTime::create(3.0f),
            CCCallFunc::create([this]() { cooldown = false; }),
            nullptr
        ));
    }

    bool init(GJGameLevel* level, bool unk) {
        if (!PlayLayer::init(level, unk)) return false;
        if (!started) {
            started = true;
            if (TEST_ON_START) {
                doJumpscare();          // 🧪 TEST: od razu po starcie
            } else {
                startRandomTimer();    // 🎯 FINAL: losowo 1/1000
            }
        }
        return true;
    }
};
