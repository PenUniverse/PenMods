// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

namespace mod {

template <typename T>
class Singleton {
public:
    static inline T& getInstance() {
        static std::unique_ptr<T> instance = nullptr;
        if (!instance) {
            instance.reset(new T());
        }
        return *instance;
    }

    static inline void createInstance() { getInstance(); }

    Singleton(const Singleton&)            = delete;
    Singleton& operator=(const Singleton&) = delete;

    Singleton(const Singleton&&)             = delete;
    Singleton&& operator=(const Singleton&&) = delete;

protected:
    explicit Singleton() = default;
    virtual ~Singleton() = default;
};

} // namespace mod
