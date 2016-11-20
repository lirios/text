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

void LanguageContext::resolveCircularDeps(QList<LanguageContext *> stack, QSharedPointer<LanguageContext> context) {
    if(context->type == Simple) {
        auto simple = context.staticCast<LanguageContextSimple>();
        for (auto inc : simple->includes) {
            if(stack.contains(inc.data())) {
                simple->includes.removeOne(inc);
                return;
            }
            auto newStack = stack;
            newStack.append(inc.data());
            resolveCircularDeps(newStack, inc);
        }
    }
    if(context->type == Container) {
        auto container = context.staticCast<LanguageContextContainer>();
        for (auto inc : container->includes) {
            if(stack.contains(inc.data())) {
                container->includes.removeOne(inc);
                return;
            }
            auto newStack = stack;
            newStack.append(inc.data());
            resolveCircularDeps(newStack, inc);
        }
    }
}
