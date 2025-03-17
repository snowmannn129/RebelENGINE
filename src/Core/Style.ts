export interface StyleProperties {
    // Colors
    backgroundColor?: string;
    foregroundColor?: string;
    borderColor?: string;
    
    // Borders
    borderWidth?: number;
    borderRadius?: number;
    borderStyle?: 'none' | 'solid' | 'dashed' | 'dotted';
    
    // Shadows
    shadowColor?: string;
    shadowBlur?: number;
    shadowOffsetX?: number;
    shadowOffsetY?: number;
    
    // Font
    fontFamily?: string;
    fontSize?: number;
    fontWeight?: 'normal' | 'bold' | number;
    fontStyle?: 'normal' | 'italic';
    textAlign?: 'left' | 'center' | 'right';
    
    // Spacing
    padding?: number;
    
    // Opacity and visibility
    opacity?: number;
    
    // Transitions
    transition?: {
        property: keyof StyleProperties;
        duration: number;
        timingFunction: 'linear' | 'ease' | 'ease-in' | 'ease-out' | 'ease-in-out';
    }[];
}

export interface StyleState {
    default: StyleProperties;
    hover?: StyleProperties;
    active?: StyleProperties;
    disabled?: StyleProperties;
}

export class Style {
    private properties: StyleState;
    private parent: Style | null;
    private currentState: keyof StyleState;
    
    constructor(properties: StyleState = { default: {} }, parent: Style | null = null) {
        this.properties = properties;
        this.parent = parent;
        this.currentState = 'default';
    }
    
    public setProperties(properties: Partial<StyleState>): void {
        this.properties = { ...this.properties, ...properties };
    }
    
    public getComputedStyle(): StyleProperties {
        const parentStyle = this.parent?.getComputedStyle() || {};
        const currentStateStyle = this.properties[this.currentState] || {};
        
        return {
            ...parentStyle,
            ...this.properties.default,
            ...currentStateStyle
        };
    }
    
    public setState(state: keyof StyleState): void {
        if (this.properties[state]) {
            this.currentState = state;
        }
    }
    
    public getState(): keyof StyleState {
        return this.currentState;
    }
    
    public setParent(parent: Style | null): void {
        this.parent = parent;
    }
    
    public getParent(): Style | null {
        return this.parent;
    }
}

export class Theme {
    private styles: Map<string, StyleState>;
    
    constructor() {
        this.styles = new Map();
    }
    
    public addStyle(name: string, style: StyleState): void {
        this.styles.set(name, style);
    }
    
    public getStyle(name: string): StyleState | undefined {
        return this.styles.get(name);
    }
    
    public removeStyle(name: string): void {
        this.styles.delete(name);
    }
}

export class StyleManager {
    private static instance: StyleManager;
    private themes: Map<string, Theme>;
    private activeTheme: string;
    
    private constructor() {
        this.themes = new Map();
        this.activeTheme = 'default';
        this.initializeDefaultTheme();
    }
    
    public static getInstance(): StyleManager {
        if (!StyleManager.instance) {
            StyleManager.instance = new StyleManager();
        }
        return StyleManager.instance;
    }
    
    private initializeDefaultTheme(): void {
        const defaultTheme = new Theme();
        
        // Panel default style
        defaultTheme.addStyle('panel', {
            default: {
                backgroundColor: '#2A2A2A',
                borderColor: '#404040',
                borderWidth: 1,
                borderStyle: 'solid',
                borderRadius: 4,
                shadowColor: 'rgba(0, 0, 0, 0.2)',
                shadowBlur: 4,
                shadowOffsetX: 0,
                shadowOffsetY: 2
            }
        });
        
        // Button styles
        defaultTheme.addStyle('button', {
            default: {
                backgroundColor: '#4A4A4A',
                foregroundColor: '#FFFFFF',
                borderRadius: 4,
                padding: 8,
                fontSize: 14,
                fontWeight: 'normal',
                transition: [
                    { property: 'backgroundColor', duration: 200, timingFunction: 'ease' }
                ]
            },
            hover: {
                backgroundColor: '#5A5A5A'
            },
            active: {
                backgroundColor: '#3A3A3A'
            },
            disabled: {
                backgroundColor: '#3A3A3A',
                foregroundColor: '#808080',
                opacity: 0.5
            }
        });
        
        this.themes.set('default', defaultTheme);
    }
    
    public addTheme(name: string, theme: Theme): void {
        this.themes.set(name, theme);
    }
    
    public getTheme(name: string): Theme | undefined {
        return this.themes.get(name);
    }
    
    public setActiveTheme(name: string): void {
        if (this.themes.has(name)) {
            this.activeTheme = name;
        }
    }
    
    public getActiveTheme(): Theme {
        return this.themes.get(this.activeTheme) || this.themes.get('default')!;
    }
    
    public createStyle(themeName: string = this.activeTheme, styleName: string, parent: Style | null = null): Style {
        const theme = this.themes.get(themeName);
        if (!theme) {
            throw new Error(`Theme '${themeName}' not found`);
        }
        
        const styleState = theme.getStyle(styleName);
        if (!styleState) {
            throw new Error(`Style '${styleName}' not found in theme '${themeName}'`);
        }
        
        return new Style(styleState, parent);
    }
}
