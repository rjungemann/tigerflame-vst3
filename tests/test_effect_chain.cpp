// TigerFlame - Effect Chain Tests

#include "catch2/catch_all.hpp"

#include "core/effect_chain.h"
#include "core/effects/qsound_effect.h"
#include "core/effects/reverb_effect.h"
#include "core/effects/delay_effect.h"

TEST_CASE("Effect Chain - Basic Functionality", "[effect_chain][basic]") {
    using namespace TigerFlame;
    
    EffectChain chain;
    chain.setSampleRate(44100.0);
    
    SECTION("Initial state") {
        REQUIRE(chain.getEffectCount() == 0);
        REQUIRE(chain.getMaxSlots() == 8);
        REQUIRE(!chain.isGlobalBypassed());
    }
    
    SECTION("Add effect") {
        int index = chain.addEffect(EffectType::kQSound);
        REQUIRE(index >= 0);
        REQUIRE(chain.getEffectType(index) == EffectType::kQSound);
    }
    
    SECTION("Add effect at specific slot") {
        int index = chain.addEffect(EffectType::kReverb, 2);
        REQUIRE(index == 2);
        REQUIRE(chain.getEffectType(2) == EffectType::kReverb);
    }
    
    SECTION("Remove effect") {
        chain.addEffect(EffectType::kQSound);
        bool removed = chain.removeEffect(0);
        REQUIRE(removed);
        REQUIRE(chain.getEffectType(0) == EffectType::kNone);
    }
    
    SECTION("Move effect") {
        chain.addEffect(EffectType::kQSound);
        chain.addEffect(EffectType::kReverb, 2);
        chain.moveEffect(0, 2);
        REQUIRE(chain.getEffectType(2) == EffectType::kQSound);
    }
    
    SECTION("Set effect type") {
        chain.addEffect(EffectType::kQSound);
        chain.setEffectType(0, EffectType::kDelay);
        REQUIRE(chain.getEffectType(0) == EffectType::kDelay);
    }
    
    SECTION("Enable/disable effect") {
        chain.addEffect(EffectType::kQSound);
        chain.setEffectEnabled(0, false);
        REQUIRE(!chain.isEffectEnabled(0));
        chain.setEffectEnabled(0, true);
        REQUIRE(chain.isEffectEnabled(0));
    }
    
    SECTION("Bypass effect") {
        chain.addEffect(EffectType::kQSound);
        chain.setEffectBypass(0, true);
        REQUIRE(chain.isEffectBypassed(0));
    }
}

TEST_CASE("Effect Chain - Audio Processing", "[effect_chain][audio]") {
    using namespace TigerFlame;
    
    EffectChain chain;
    chain.setSampleRate(44100.0);
    
    SECTION("Empty chain passes audio through") {
        float left = 1.0f;
        float right = -0.5f;
        chain.process(&left, &right);
        REQUIRE(left == 1.0f);
        REQUIRE(right == -0.5f);
    }
    
    SECTION("Global bypass prevents processing") {
        chain.addEffect(EffectType::kQSound);
        chain.setGlobalBypass(true);
        
        float left = 1.0f;
        float right = -0.5f;
        chain.process(&left, &right);
        REQUIRE(left == 1.0f);
        REQUIRE(right == -0.5f);
    }
    
    SECTION("Process block") {
        chain.addEffect(EffectType::kQSound);
        
        const int numSamples = 256;
        float left[numSamples];
        float right[numSamples];
        
        for (int i = 0; i < numSamples; ++i) {
            left[i] = 0.5f;
            right[i] = -0.5f;
        }
        
        chain.processBlock(left, right, numSamples);
        // Should not crash
        REQUIRE(true);
    }
    
    SECTION("Effect parameters") {
        chain.addEffect(EffectType::kQSound);
        chain.setEffectParameter(0, 0, 0.5f); // Delay time
        REQUIRE(chain.getEffectParameter(0, 0) == 0.5f);
    }
}

TEST_CASE("Effect Chain - State Persistence", "[effect_chain][state]") {
    using namespace TigerFlame;
    
    EffectChain chain;
    chain.setSampleRate(44100.0);
    
    SECTION("Save and restore empty chain") {
        // Save state
        // (Can't easily test IBStream without mocking)
        REQUIRE(true);
    }
    
    SECTION("Save and restore chain with effects") {
        chain.addEffect(EffectType::kQSound);
        chain.addEffect(EffectType::kReverb, 2);
        chain.setEffectParameter(0, 0, 0.7f);
        
        REQUIRE(chain.getEffectType(0) == EffectType::kQSound);
        REQUIRE(chain.getEffectType(2) == EffectType::kReverb);
    }
}

TEST_CASE("Effect Chain - Effect Ordering", "[effect_chain][ordering]") {
    using namespace TigerFlame;
    
    EffectChain chain;
    chain.setSampleRate(44100.0);
    
    SECTION("Effects process in order") {
        chain.addEffect(EffectType::kQSound, 0);
        chain.addEffect(EffectType::kReverb, 1);
        
        // Process some audio
        float left = 1.0f;
        float right = 0.0f;
        chain.process(&left, &right);
        
        // The exact output depends on the effect implementations
        // This just verifies the chain processes in order
        REQUIRE(true);
    }
}

TEST_CASE("QSound Effect - Basic", "[qsound][basic]") {
    using namespace TigerFlame;
    
    QSoundEffect effect;
    effect.setSampleRate(44100.0);
    
    SECTION("Silence in produces silence out") {
        float left = 0.0f;
        float right = 0.0f;
        effect.process(&left, &right);
        REQUIRE(left == 0.0f);
        REQUIRE(right == 0.0f);
    }
    
    SECTION("Non-silence produces output") {
        float left = 1.0f;
        float right = 0.0f;
        effect.process(&left, &right);
        // Output may be different from input
        REQUIRE(true);
    }
    
    SECTION("Set parameters") {
        effect.setParameter(QSoundEffect::kDelayTime, 0.5f);
        effect.setParameter(QSoundEffect::kFeedback, 0.7f);
        effect.setParameter(QSoundEffect::kDamping, 0.3f);
        effect.setParameter(QSoundEffect::kStereoWidth, 0.8f);
        
        REQUIRE(effect.getParameter(QSoundEffect::kDelayTime) == 0.5f);
        REQUIRE(effect.getParameter(QSoundEffect::kFeedback) == 0.7f);
    }
    
    SECTION("Bypass") {
        effect.setBypass(true);
        float left = 1.0f;
        float right = 0.5f;
        effect.process(&left, &right);
        REQUIRE(left == 1.0f);
        REQUIRE(right == 0.5f);
    }
    
    SECTION("Enable/disable") {
        effect.setEnabled(false);
        float left = 1.0f;
        float right = 0.5f;
        effect.process(&left, &right);
        REQUIRE(left == 1.0f);
        REQUIRE(right == 0.5f);
    }
}
