import React from 'react';
import { Box, Drawer, List, ListItem, ListItemText, AppBar, Toolbar, Typography } from '@mui/material';
import { useNavigate } from 'react-router-dom';

const drawerWidth = 240;

const sections = [
  'RenderingSystem',
  'PhysicsSystem',
  'AnimationSystem',
  'AudioSystem',
  'InputSystem',
  'ScriptingAndLogicSystem',
  'ArtificialIntelligence',
  'NetworkingAndMultiplayer',
  'AssetManagement',
  'UIAndUserInteraction',
  'GameplayMechanics',
  'WorldBuildingAndProceduralGeneration',
  'DebuggingAndProfilingTools',
  'VRAndARSupport',
  'ModdingTools',
  'PluginAndExtensionSupport',
  'DocumentationAndStandards'
];

const Layout: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const navigate = useNavigate();

  return (
    <Box sx={{ display: 'flex' }}>
      <AppBar position="fixed" sx={{ zIndex: (theme) => theme.zIndex.drawer + 1 }}>
        <Toolbar>
          <Typography variant="h6" noWrap component="div">
            Game Engine Documentation
          </Typography>
        </Toolbar>
      </AppBar>
      <Drawer
        variant="permanent"
        sx={{
          width: drawerWidth,
          flexShrink: 0,
          '& .MuiDrawer-paper': {
            width: drawerWidth,
            boxSizing: 'border-box',
          },
        }}
      >
        <Toolbar />
        <Box sx={{ overflow: 'auto' }}>
          <List>
            {sections.map((section) => (
              <ListItem 
                key={section}
                onClick={() => {
                  const defaultFiles: Record<string, string> = {
                    'RenderingSystem': 'scenerendering',
                    'PhysicsSystem': 'rigidbodyphysics',
                    'AnimationSystem': 'skeletalanimation',
                    'AudioSystem': 'soundeffectsmanagement',
                    'InputSystem': 'keyboardandmouseinput',
                    'ScriptingAndLogicSystem': 'scriptexecution',
                    'ArtificialIntelligence': 'pathfinding',
                    'NetworkingAndMultiplayer': 'clientservercommunication',
                    'AssetManagement': 'resourceloader',
                    'UIAndUserInteraction': 'hudsystem',
                    'GameplayMechanics': 'gamerulesandlogic',
                    'WorldBuildingAndProceduralGeneration': 'terraingeneration',
                    'DebuggingAndProfilingTools': 'performanceprofiler',
                    'VRAndARSupport': 'headmounteddisplayintegration',
                    'ModdingTools': 'moddingsdk',
                    'PluginAndExtensionSupport': 'pluginarchitecture',
                    'DocumentationAndStandards': 'apidocumentation'
                  };
                  navigate(`/${section.toLowerCase()}/${defaultFiles[section].toLowerCase()}`);
                }}
                sx={{ cursor: 'pointer' }}
              >
                <ListItemText primary={section} />
              </ListItem>
            ))}
          </List>
        </Box>
      </Drawer>
      <Box component="main" sx={{ flexGrow: 1, p: 3 }}>
        <Toolbar />
        {children}
      </Box>
    </Box>
  );
};

export default Layout;
