/**
 * Mock Electron IPC for testing
 */
class MockIpcRenderer {
  constructor() {
    this.handlers = new Map();
    this.listeners = new Map();
  }

  handle(channel, fn) {
    this.handlers.set(channel, fn);
  }

  async invoke(channel, ...args) {
    const handler = this.handlers.get(channel);
    if (!handler) throw new Error(`No mock handler registered for IPC channel: ${channel}`);
    return handler({}, ...args);
  }

  on(channel, fn) {
    if (!this.listeners.has(channel)) this.listeners.set(channel, []);
    this.listeners.get(channel).push(fn);
  }

  emit(channel, ...args) {
    const list = this.listeners.get(channel) || [];
    for (const fn of list) fn({}, ...args);
  }
}

module.exports = {
  MockIpcRenderer
};
