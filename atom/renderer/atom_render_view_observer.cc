// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/renderer/atom_render_view_observer.h"

#include <vector>

#include "atom/common/api/api_messages.h"
#include "atom/renderer/api/atom_renderer_bindings.h"
#include "atom/renderer/atom_renderer_client.h"
#include "content/public/renderer/render_view.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/web/WebDraggableRegion.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebView.h"

#include "atom/common/node_includes.h"

using WebKit::WebFrame;

namespace atom {

AtomRenderViewObserver::AtomRenderViewObserver(
    content::RenderView* render_view,
    AtomRendererClient* renderer_client)
    : content::RenderViewObserver(render_view),
      renderer_client_(renderer_client) {
}

AtomRenderViewObserver::~AtomRenderViewObserver() {
}

void AtomRenderViewObserver::DraggableRegionsChanged(WebKit::WebFrame* frame) {
  WebKit::WebVector<WebKit::WebDraggableRegion> webregions =
      frame->document().draggableRegions();
  std::vector<DraggableRegion> regions;
  for (size_t i = 0; i < webregions.size(); ++i) {
    DraggableRegion region;
    region.bounds = webregions[i].bounds;
    region.draggable = webregions[i].draggable;
    regions.push_back(region);
  }
  Send(new AtomViewHostMsg_UpdateDraggableRegions(routing_id(), regions));
}

bool AtomRenderViewObserver::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(AtomRenderViewObserver, message)
    IPC_MESSAGE_HANDLER(AtomViewMsg_Message, OnBrowserMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void AtomRenderViewObserver::OnBrowserMessage(const string16& channel,
                                              const base::ListValue& args) {
  if (!render_view()->GetWebView())
    return;

  WebKit::WebFrame* frame = render_view()->GetWebView()->mainFrame();
  if (!renderer_client_->IsNodeBindingEnabled(frame))
    return;

  renderer_client_->atom_bindings()->OnBrowserMessage(
      render_view(), channel, args);
}

}  // namespace atom
