open Rebase;

open Core;

open Dom;

[%bs.raw {|require('../../../../src/popup.html')|}];

[%bs.raw {|require('../../../../src/images/logo19.png')|}];

[%bs.raw {|require('../../../../src/images/logo38.png')|}];

[%bs.raw {|require('../../../../src/images/logo128.png')|}];

[%bs.raw {|require('../../../../src/css/codemirror.css')|}];

[%bs.raw {|require('../../../../src/css/oceanic-next.css')|}];

[%bs.raw {|require('codemirror/mode/javascript/javascript')|}];

[%bs.raw {|require('codemirror/mode/mllike/mllike')|}];

let open_: string => unit = [%bs.raw
  {|
  function (hash) {
    window.open(
      window.location.href,
      "_blank"
    );
  }
|}
];

let setHash hash => Core.Hisory.replaceState state::[%bs.raw "{}"] title::"" url::hash;

exception DeserializationFail;

type request = {input: string};

type payload = {outText: string, inLang: string, outLang: string};

let makeContentHash text => "#" ^ Util.btoa text;

let generateShareableLink text => "https://reasonml.github.io/reason-tools/popup.html" ^ text;

let getInputFromUrl () => {
  let text = Location.hash |> Js.String.sliceToEnd from::1 |> Util.atob;
  if (Str.isEmpty text) {
    Promise.reject ()
  } else {
    Promise.resolve text
  }
};

let rec inputChanged input => {
  let hash = makeContentHash input;
  let link = generateShareableLink hash;
  /* this isn't guaranteed to be sync or speedy, so
   * don't set this.state.in here, since it could cause lag.
   */
  ignore (
    ReasonJs.setTimeout
      (
        fun () => {
          switch (Background.Refmt.refmt input) {
          | ("Failure", error) => render input error None None link
          | (conversion, outText) =>
            switch (conversion |> Js.String.split "to") {
            | [|inLang, outLang|] => render input outText (Some inLang) (Some outLang) link
            | _ => ()
            }
          };
          setHash hash
        }
      )
      0
  )
}
and render inText outText inLang outLang link =>
  ReactDOMRe.render
    <PopupWindow inText inLang outText outLang link onOpen=open_ onInputChanged=inputChanged />
    (ReasonJs.Document.getElementById "app");

let init _ => Promise.(getInputFromUrl () |> or_ (fun _ => "") |> then_ inputChanged |> ignore);

Document.addEventListener "DOMContentLoaded" init;
