/**
 * Mock Fetch Helper for YouTube Data API v3 Responses
 */
function createMockFetch(responses = {}) {
  return async function mockFetch(url, options = {}) {
    const urlStr = url.toString();

    for (const [endpointPattern, responseConfig] of Object.entries(responses)) {
      if (urlStr.includes(endpointPattern)) {
        if (responseConfig.throwNetworkError) {
          throw new Error('Network failure: Unable to reach YouTube API');
        }

        return {
          ok: responseConfig.status >= 200 && responseConfig.status < 300,
          status: responseConfig.status || 200,
          json: async () => responseConfig.data || {}
        };
      }
    }

    return {
      ok: true,
      status: 200,
      json: async () => ({ items: [] })
    };
  };
}

module.exports = {
  createMockFetch
};
