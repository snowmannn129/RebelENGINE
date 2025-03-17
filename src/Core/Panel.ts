import { UIControl, Slider, Button, Checkbox, ColorPicker, Dropdown } from './UIControls';
import { Layout, LayoutType, Alignment, Spacing, GridConfig } from './Layout';
import { Style, StyleManager, StyleProperties } from './Style';

export class Panel {
    protected name: string;
    protected visible: boolean;
    protected position: { x: number; y: number };
    protected size: { width: number; height: number };
    protected children: (Panel | UIControl)[];
    protected layout: Layout;
    protected style: Style;
    protected styleManager: StyleManager;

    constructor(name: string) {
        this.name = name;
        this.visible = true;
        this.position = { x: 0, y: 0 };
        this.size = { width: 300, height: 200 };
        this.children = [];
        this.layout = new Layout();
        this.styleManager = StyleManager.getInstance();
        this.style = this.styleManager.createStyle('default', 'panel');
    }

    public getStyle(): Style {
        return this.style;
    }

    public setStyle(style: Style): void {
        this.style = style;
        this.refresh();
    }

    public applyStyleProperties(properties: Partial<StyleProperties>): void {
        this.style.setProperties({ default: properties });
        this.refresh();
    }

    public setStyleState(state: 'default' | 'hover' | 'active' | 'disabled'): void {
        this.style.setState(state);
        this.refresh();
    }

    public getName(): string {
        return this.name;
    }

    public isVisible(): boolean {
        return this.visible;
    }

    public show(): void {
        this.visible = true;
    }

    public hide(): void {
        this.visible = false;
    }

    public setPosition(x: number, y: number): void {
        this.position = { x, y };
    }

    public getPosition(): { x: number; y: number } {
        return this.position;
    }

    public setSize(width: number, height: number): void {
        this.size = { width, height };
    }

    public getSize(): { width: number; height: number } {
        return this.size;
    }

    public addChild(child: Panel | UIControl): void {
        this.children.push(child);
    }

    public removeChild(child: Panel | UIControl): void {
        const index = this.children.indexOf(child);
        if (index !== -1) {
            this.children.splice(index, 1);
        }
    }

    public getChildren(): (Panel | UIControl)[] {
        return this.children;
    }

    protected addGroup(name: string): PanelGroup {
        const group = new PanelGroup(name);
        this.addChild(group);
        return group;
    }

    public getLayout(): Layout {
        return this.layout;
    }

    public setLayoutType(type: LayoutType): void {
        this.layout.setType(type);
        this.refresh();
    }

    public setAlignment(alignment: Alignment): void {
        this.layout.setAlignment(alignment);
        this.refresh();
    }

    public setMargin(spacing: Partial<Spacing>): void {
        this.layout.setMargin(spacing);
        this.refresh();
    }

    public setPadding(spacing: Partial<Spacing>): void {
        this.layout.setPadding(spacing);
        this.refresh();
    }

    public setGridConfig(config: GridConfig): void {
        this.layout.setGridConfig(config);
        this.refresh();
    }

    public refresh(): void {
        // Apply current style
        const computedStyle = this.style.getComputedStyle();
        
        // Apply style properties to the panel's visual representation
        // These would be used by the rendering system
        Object.assign(this, {
            backgroundColor: computedStyle.backgroundColor,
            foregroundColor: computedStyle.foregroundColor,
            borderColor: computedStyle.borderColor,
            borderWidth: computedStyle.borderWidth,
            borderRadius: computedStyle.borderRadius,
            borderStyle: computedStyle.borderStyle,
            shadowColor: computedStyle.shadowColor,
            shadowBlur: computedStyle.shadowBlur,
            shadowOffsetX: computedStyle.shadowOffsetX,
            shadowOffsetY: computedStyle.shadowOffsetY,
            opacity: computedStyle.opacity,
            padding: computedStyle.padding || 0
        });

        // Calculate and apply new positions for children based on layout
        const childSizes = this.children.map(child => ({
            size: child.getSize()
        }));
        const positions = this.layout.calculateChildPositions(this.size, childSizes);
        
        // Update children positions and refresh their state
        this.children.forEach((child, index) => {
            child.setPosition(positions[index].x, positions[index].y);
            child.refresh();
        });
    }
}

export class PanelGroup extends Panel {
    private expanded: boolean;

    constructor(name: string) {
        super(name);
        this.expanded = true;
    }

    public isExpanded(): boolean {
        return this.expanded;
    }

    public expand(): void {
        this.expanded = true;
    }

    public collapse(): void {
        this.expanded = false;
    }

    public toggle(): void {
        this.expanded = !this.expanded;
    }

    public addSlider(label: string, value: number, min: number, max: number, onChange?: (value: number) => void): Slider {
        const slider = new Slider(label, value, min, max, onChange);
        this.children.push(slider);
        return slider;
    }

    public addButton(label: string, callback: () => void): Button {
        const button = new Button(label, callback);
        this.children.push(button);
        return button;
    }

    public addCheckbox(label: string, checked: boolean = false, onChange?: (checked: boolean) => void): Checkbox {
        const checkbox = new Checkbox(label, checked, onChange);
        this.children.push(checkbox);
        return checkbox;
    }

    public addColorPicker(label: string, color: string = '#000000', onChange?: (color: string) => void): ColorPicker {
        const colorPicker = new ColorPicker(label, color, onChange);
        this.children.push(colorPicker);
        return colorPicker;
    }

    public addDropdown(label: string, options: string[], selectedIndex: number = 0, onChange?: (selectedIndex: number, selectedValue: string) => void): Dropdown {
        const dropdown = new Dropdown(label, options, selectedIndex, onChange);
        this.children.push(dropdown);
        return dropdown;
    }
}
