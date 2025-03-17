import React, { Suspense } from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate, useParams } from 'react-router-dom';
import Layout from './components/Layout';
import MarkdownViewer from './components/MarkdownViewer';
import ErrorBoundary from './components/ErrorBoundary';
import './App.css';

type RouteParams = {
  section?: string;
  subsection?: string;
}

const LoadingFallback = () => (
  <div role="progressbar" aria-label="Loading content">
    Loading...
  </div>
);

const MarkdownContent: React.FC = () => {
  const { section, subsection } = useParams();

  if (!section || !subsection) {
    return <Navigate to="/renderingsystem/scenerendering" replace />;
  }

  const filePath = `${section}/${subsection}.md`;

  return (
    <Suspense fallback={<LoadingFallback />}>
      <MarkdownViewer filePath={filePath.toLowerCase()} />
    </Suspense>
  );
};

const App: React.FC = () => (
  <ErrorBoundary>
    <Router>
      <div className="app-container">
        <Layout>
          <Suspense fallback={<LoadingFallback />}>
            <Routes>
              <Route path="/" element={<Navigate to="/renderingsystem/scenerendering" replace />} />
              <Route path="/:section/:subsection" element={<MarkdownContent />} />
              <Route path="*" element={<Navigate to="/renderingsystem/scenerendering" replace />} />
            </Routes>
          </Suspense>
        </Layout>
      </div>
    </Router>
  </ErrorBoundary>
);

export default App;
