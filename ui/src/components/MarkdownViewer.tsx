import React, { useEffect, useState } from 'react';
import ReactMarkdown from 'react-markdown';
import { Box, Paper, Typography } from '@mui/material';
import '../styles/markdown.css';

interface MarkdownViewerProps {
  filePath: string;
}

const MarkdownViewer: React.FC<MarkdownViewerProps> = ({ filePath }) => {
  const [content, setContent] = useState<string>('');
  const [error, setError] = useState<string>('');

  useEffect(() => {
    const fetchContent = async () => {
      try {
        console.log('Fetching file:', filePath);
        const response = await fetch(`/${filePath.toLowerCase()}`);
        if (!response.ok) {
          throw new Error(`Failed to load content: ${response.status} ${response.statusText}`);
        }
        const text = await response.text();
        setContent(text);
        setError('');
      } catch (err) {
        setError('Failed to load markdown content');
        setContent('');
      }
    };

    fetchContent();
  }, [filePath]);

  if (error) {
    return (
      <Box sx={{ p: 2 }}>
        <Typography color="error">{error}</Typography>
      </Box>
    );
  }

  return (
    <Paper elevation={2} sx={{ p: 3 }}>
      <div className="markdown-content">
        <ReactMarkdown>{content}</ReactMarkdown>
      </div>
    </Paper>
  );
};

export default MarkdownViewer;
