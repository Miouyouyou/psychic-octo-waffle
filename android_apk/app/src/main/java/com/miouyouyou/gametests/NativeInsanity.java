package com.miouyouyou.gametests;

public class NativeInsanity extends android.app.NativeActivity {
  static { System.loadLibrary("main"); }
}
