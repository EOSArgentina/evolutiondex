"use strict";

/**
  Convert a synchronous function into a asynchronous one (via setTimeout)
  wrapping it in a promise.  This does not expect the function to have a
  callback paramter.

  @arg {function} func - non-callback function

  @example promiseAsync(myfunction)
*/
module.exports = function (func) {
  return function () {
    for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
      args[_key] = arguments[_key];
    }

    return new Promise(function (resolve, reject) {
      setTimeout(function () {
        try {
          resolve(func.apply(void 0, args));
        } catch (err) {
          reject(err);
        }
      });
    });
  };
};