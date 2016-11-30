/*
 * Copyright © 2016 Andrew Penkrat
 *
 * This file is part of Liri Text.
 *
 * Liri Text is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Liri Text is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Liri Text.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "languagecontext.h"
#include "languagecontextcontainer.h"
#include "languagecontextsimple.h"

LanguageContext::LanguageContext(LanguageContext::ElementType t) :
    type(t),
    style(nullptr) { }

LanguageContext::~LanguageContext() { }

void LanguageContext::resolveCircularDeps(QList<LanguageContext *> stack) {
    auto current = stack.last();
    if(current->type == Simple) {
        auto simple = static_cast<LanguageContextSimple *>(current);
        resolveCircularDeps(simple, stack);
    }
    if(current->type == Container) {
        auto container = static_cast<LanguageContextContainer *>(current);
        resolveCircularDeps(container, stack);
    }
}

template<class ContextType>
void LanguageContext::resolveCircularDeps(ContextType *current, QList<LanguageContext *> stack) {
    for (auto inc : current->includes) {
        if(stack.contains(inc->data())) {
            current->includes.removeOne(inc);
            continue;
        }
        auto newStack = stack;
        newStack.append(inc->data());
        resolveCircularDeps(newStack);
    }
}

template<class ContextType>
void LanguageContext::resolveCircularDeps(ContextType *current) {
    resolveCircularDeps(current, QList<LanguageContext *>({current}));
}

template void LanguageContext::resolveCircularDeps<LanguageContextContainer>(LanguageContextContainer *current, QList<LanguageContext *> stack);
template void LanguageContext::resolveCircularDeps<LanguageContextSimple>(LanguageContextSimple *current, QList<LanguageContext *> stack);

template void LanguageContext::resolveCircularDeps<LanguageContextContainer>(LanguageContextContainer *current);
template void LanguageContext::resolveCircularDeps<LanguageContextSimple>(LanguageContextSimple *current);
