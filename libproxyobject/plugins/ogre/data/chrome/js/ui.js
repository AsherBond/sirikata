/*  Sirikata
 *  ui.js
 *
 *  Copyright (c) 2011, Stanford University
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Sirikata nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/* This file contains utility methods for . A few C++ level helpers
 * are generated automatically in WebView.cpp, which this file relies
 * on. Therefore, the sirikata global object already exists.
 */

/** Log data to the console. The first parameter should be a log
 * level, i.e. 'fatal', 'error', 'warn', 'debug', 'info', etc. The
 * remaining arguments are converted to strings and printed, separated
 * by spaces.
 */
sirikata.log = function() {
    var args = [];
    for(var i = 0; i < arguments.length; i++) { args.push(arguments[i]); }
    sirikata.event.apply(this, ['__log'].concat(args));
};

/** A wrapper for UI code which sets up the environment for
 * isolation. You should generally execute all your UI code through
 * this, e.g. your UI script should look like this:
 *
 *   sirikata.ui('my-module', function() {
 *     real ui code
 *   });
 */
sirikata.ui = function(name, ui_code) {
    $(document).ready(
        function() {
            var sirikata = {};
            for(var i in __sirikata) { sirikata[i] = __sirikata[i]; }
            sirikata.event = function() {
                var args = [];
                for(var i = 0; i < arguments.length; i++) { args.push(arguments[i]); }
                return __sirikata.event.apply(this, [name + '-' + args[0]].concat(args.slice(1)) );
            };
            eval('(' + ui_code.toString() + ')()');
        }
    );
};

/** Create a UI window from the given element.
 *  @param selector selector or DOM element to convert into a window
 *  @param params settings for this window. Currently this mainly just
 *         passes through to jQuery UI, but the settings may change in the
 *         future.
 */
sirikata.ui.window = function(selector, params) {
    var win_data = $(selector);
    var real_params = {};
    for(var i in params) {
        if (i == 'modal') continue;
        real_params[i] = params[i];
    }
    return win_data.dialog(real_params);
};

/** Use the given element like a button. Allows you to do things like set a click handler.
 *  @param selector selector or DOM element to use as button.
 */
sirikata.ui.button = function(selector) {
    return $(selector);
};